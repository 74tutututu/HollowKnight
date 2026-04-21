#include "Spike.h"

USING_NS_CC;

Spike* Spike::create(const std::string& filename)
{
    Spike* spike = new (std::nothrow) Spike();
    if (spike && spike->initWithFile(filename) && spike->init())
    {
        spike->autorelease();
        CCLOG("? [Spike::create] Succeeded with file: %s", filename.c_str());
        return spike;
    }
    CCLOG("? [Spike::create] FAILED with file: %s", filename.c_str());
    CC_SAFE_DELETE(spike);
    return nullptr;
}

bool Spike::init()
{
    // 【修复】不调用Sprite::init()，因为它会清空initWithFile()加载的纹理
    // 直接初始化成员变量即可

    // 初始化状态
    _currentState = State::IDLE;
    
    // 检测范围（水平距离）
    _detectionRange = 50.0f;  // 玩家在刺下方50像素范围内会触发
    
    // 物理属性
    _velocity = Vec2::ZERO;
    _gravity = 2000.0f;         // 使用与Player/Zombie类似的重力值
    _maxFallSpeed = -1500.0f;   // 最大下落速度
    
    // 伤害值
    _damage = 1;
    
    // 初始位置（稍后通过setInitialPosition设置）
    _initialPosition = Vec2::ZERO;

    CCLOG("? [Spike::init] Spike initialized successfully!");

    return true;
}

void Spike::update(float dt, const cocos2d::Vec2& playerPos, const std::vector<cocos2d::Rect>& platforms)
{
    if (_currentState == State::DEAD)
    {
        return;
    }

    // 【调试】每秒输出一次刺的状态信息
    static float debugTimer = 0;
    debugTimer += dt;
    if (debugTimer >= 1.0f)
    {
        debugTimer = 0;
        Vec2 spikePos = this->getPosition();
    //  CCLOG("[Spike Debug] Position: (%.1f, %.1f), State: %d, Visible: %d, PlayerPos: (%.1f, %.1f)", 
       //       spikePos.x, spikePos.y, (int)_currentState, this->isVisible(), playerPos.x, playerPos.y);
    }

    // 根据状态执行不同行为
    switch (_currentState)
    {
    case State::IDLE:
        // 检测玩家是否在刺的下方
        if (isPlayerBelowSpike(playerPos))
        {
            CCLOG("[Spike] Player detected below! Falling!");
            changeState(State::FALLING);
        }
        break;

    case State::FALLING:
        // 应用重力下落
        updateMovementY(dt);
        updateCollisionY(platforms);
        break;

    default:
        break;
    }
}

bool Spike::isPlayerBelowSpike(const cocos2d::Vec2& playerPos)
{
    Vec2 spikePos = this->getPosition();
    
    // 检查玩家是否在刺的下方且在水平检测范围内
    float horizontalDistance = std::abs(playerPos.x - spikePos.x);
    bool isInHorizontalRange = horizontalDistance <= _detectionRange;
    
    // 检查玩家是否在刺的下方（Y坐标小于刺）
    bool isBelow = playerPos.y < spikePos.y;
    
    return isInHorizontalRange && isBelow;
}

void Spike::changeState(State newState)
{
    if (_currentState == newState)
    {
        return;
    }

    _currentState = newState;
    
    switch (_currentState)
    {
    case State::FALLING:
        // 开始下落时初始化速度
        _velocity.y = 0;
        CCLOG("[Spike] State changed to FALLING");
        break;

    case State::DEAD:
    {
        // 淡出并移除
        auto fadeOut = FadeOut::create(0.5f);
        auto removeSelf = CallFunc::create([this]() {
            this->removeFromParent();
            CCLOG("? Spike removed from scene");
        });
        auto sequence = Sequence::create(fadeOut, removeSelf, nullptr);
        this->runAction(sequence);
        break;
    }

    default:
        break;
    }
}

void Spike::updateMovementY(float dt)
{
    // 应用重力
    _velocity.y -= _gravity * dt;

    // 限制最大下落速度
    if (_velocity.y < _maxFallSpeed)
    {
        _velocity.y = _maxFallSpeed;
    }

    // 更新垂直位置
    float dy = _velocity.y * dt;
    this->setPositionY(this->getPositionY() + dy);
}

void Spike::updateCollisionY(const std::vector<cocos2d::Rect>& platforms)
{
    Rect spikeRect = this->getBoundingBox();

    for (const auto& platform : platforms)
    {
        if (spikeRect.intersectsRect(platform))
        {
            // 计算X轴重叠宽度
            float overlapX = std::min(spikeRect.getMaxX(), platform.getMaxX()) -
                             std::max(spikeRect.getMinX(), platform.getMinX());

            // 只有当X轴重叠足够大时，才进行Y轴修正
            if (overlapX > spikeRect.size.width * 0.3f)
            {
                // 下落中碰到地面
                if (_velocity.y <= 0)
                {
                    float tolerance = 40.0f;
                    float overlapY = platform.getMaxY() - spikeRect.getMinY();

                    if (overlapY > -0.1f && overlapY <= tolerance)
                    {
                        // 落地修正
                        float newY = platform.getMaxY() + spikeRect.size.height * 0.5f;
                        this->setPositionY(newY);
                        _velocity.y = 0;
                        
                        // 【修改】落地后立即进入DEAD状态（失去碰撞和伤害）
                        changeState(State::DEAD);
                        
                        CCLOG("[Spike] Hit ground! Changed to DEAD state (no collision, no damage)");
                    }
                }
            }
        }
    }
}

cocos2d::Rect Spike::getHitbox() const
{
    // 死亡状态不返回碰撞箱
    if (_currentState == State::DEAD)
    {
        return Rect::ZERO;
    }

    // 返回完整的碰撞箱
    return this->getBoundingBox();
}

void Spike::setInitialPosition(const cocos2d::Vec2& pos)
{
    _initialPosition = pos;
    this->setPosition(pos);
}

Spike::~Spike()
{
    CCLOG("[Spike] Destructor called");
}
