#include "Buzzer.h"
#include "HitEffect.h"

USING_NS_CC;

Buzzer* Buzzer::create(const std::string& filename)
{
    Buzzer* buzzer = new (std::nothrow) Buzzer();
    if (buzzer && buzzer->initWithFile(filename) && buzzer->init())
    {
        buzzer->autorelease();
        CCLOG("[Buzzer::create] Succeeded with file: %s", filename.c_str());
        return buzzer;
    }
    CCLOG("[Buzzer::create] FAILED with file : % s", filename.c_str());
    CC_SAFE_DELETE(buzzer);
    return nullptr;
}

bool Buzzer::init()
{
    // 不调用Sprite::init()，避免清空纹理

    // 初始化属性
    _currentState = State::IDLE;
    _health = 3;
    _maxHealth = 3;
    _isInvincible = false;

    // AI参数
    _detectionRange = 600.0f;  // 【修改】检测范围从400增加到600像素
    _chaseSpeed = 300.0f;      // 【修改】追击速度从150增加到300（两倍）
    _velocity = Vec2::ZERO;
    _initialPosition = Vec2::ZERO;

    // 加载动画
    loadAnimations();
    playIdleAnimation();

    // 设置梦之钉心声 (这是 GameEntity 提供的功能)
    this->setDreamThought("...Kill...Crush...");

    CCLOG(" [Buzzer::init] Buzzer initialized successfully!");

    return true;
}

void Buzzer::loadAnimations()
{
    // 加载idle动画 (4帧)
    Vector<SpriteFrame*> idleFrames;
    for (int i = 1; i <= 4; i++)
    {
        std::string frameName = StringUtils::format("buzzer/idle/idle_%d.png", i);
        auto sprite = Sprite::create(frameName);
        if (sprite)
        {
            idleFrames.pushBack(sprite->getSpriteFrame());
            CCLOG("  ? Loaded idle frame: %s", frameName.c_str());
        }
        else
        {
            CCLOG("  ? Failed to load idle frame: %s", frameName.c_str());
        }
    }

    if (idleFrames.size() > 0)
    {
        _idleAnimation = Animation::createWithSpriteFrames(idleFrames, 0.15f);
        _idleAnimation->retain();
        CCLOG("? Idle animation created with %d frames", (int)idleFrames.size());
    }
    else
    {
        _idleAnimation = nullptr;
    }

    // 加载attack动画 (5帧)
    Vector<SpriteFrame*> attackFrames;
    for (int i = 1; i <= 5; i++)
    {
        std::string frameName = StringUtils::format("buzzer/attack/attack_%d.png", i);
        auto sprite = Sprite::create(frameName);
        if (sprite)
        {
            attackFrames.pushBack(sprite->getSpriteFrame());
            CCLOG("  ? Loaded attack frame: %s", frameName.c_str());
        }
        else
        {
            CCLOG("  ? Failed to load attack frame: %s", frameName.c_str());
        }
    }

    if (attackFrames.size() > 0)
    {
        _attackAnimation = Animation::createWithSpriteFrames(attackFrames, 0.1f);
        _attackAnimation->retain();
        CCLOG("? Attack animation created with %d frames", (int)attackFrames.size());
    }
    else
    {
        _attackAnimation = nullptr;
    }
}

void Buzzer::playIdleAnimation()
{
    if (_idleAnimation)
    {
        this->stopActionByTag(100);
        auto animate = Animate::create(_idleAnimation);
        auto repeat = RepeatForever::create(animate);
        repeat->setTag(100);
        this->runAction(repeat);
        CCLOG("?? Playing idle animation");
    }
}

void Buzzer::playAttackAnimation()
{
    if (_attackAnimation)
    {
        this->stopActionByTag(100);
        auto animate = Animate::create(_attackAnimation);
        auto repeat = RepeatForever::create(animate);
        repeat->setTag(100);
        this->runAction(repeat);
        CCLOG("? Playing attack animation");
    }
}

void Buzzer::playDeathAnimation()
{
    this->stopAllActions();

    auto fadeOut = FadeOut::create(0.5f);
    auto rotateBy = RotateBy::create(0.5f, 360);
    auto spawn = Spawn::create(fadeOut, rotateBy, nullptr);

    auto removeSelf = CallFunc::create([this]() {
        this->removeFromParent();
        CCLOG("?? Buzzer removed from scene");
    });

    auto sequence = Sequence::create(spawn, removeSelf, nullptr);
    this->runAction(sequence);
}

// ========================================
// AI 核心更新函数
// ========================================
void Buzzer::update(float dt, const cocos2d::Vec2& playerPos)
{
    if (_currentState == State::DEAD)
    {
        return;
    }

    // 根据状态执行不同行为
    switch (_currentState)
    {
    case State::IDLE:
        // 检测玩家是否在范围内
        if (isPlayerInRange(playerPos))
        {
            CCLOG("[Buzzer] Player detected! Starting attack!");
            changeState(State::ATTACKING);
        }
        break;

    case State::ATTACKING:
        // 追击玩家
        chasePlayer(playerPos, dt);
        
        // 如果玩家离开范围，返回idle
        if (!isPlayerInRange(playerPos))
        {
            CCLOG("[Buzzer] Player out of range, returning to idle");
            changeState(State::IDLE);
        }
        break;

    default:
        break;
    }
}

bool Buzzer::isPlayerInRange(const cocos2d::Vec2& playerPos)
{
    Vec2 buzzerPos = this->getPosition();
    // 【修改】只检查水平距离（X轴），不考虑垂直距离
    float horizontalDistance = std::abs(playerPos.x - buzzerPos.x);
    return horizontalDistance <= _detectionRange;
}

void Buzzer::chasePlayer(const cocos2d::Vec2& playerPos, float dt)
{
    Vec2 buzzerPos = this->getPosition();
    
    // 【修改】升高最低高度限制（从400提升到500）
    float minHeight = 500.0f; // 地面高度
    
    // 计算到玩家的方向向量
    Vec2 direction = playerPos - buzzerPos;
    
    // 如果距离很近就不移动了
    float distance = direction.length();
    if (distance < 20.0f)
    {
        return;
    }
    
    // 【修改】如果Buzzer低于最低高度，只进行水平移动
    if (buzzerPos.y <= minHeight)
    {
        // 只水平移动
        float horizontalDistance = std::abs(playerPos.x - buzzerPos.x);
        if (horizontalDistance > 20.0f)
        {
            // 计算水平方向
            float horizontalDirection = (playerPos.x > buzzerPos.x) ? 1.0f : -1.0f;
            
            // 只应用水平速度
            Vec2 newPos = Vec2(buzzerPos.x + horizontalDirection * _chaseSpeed * dt, minHeight);
            this->setPosition(newPos);
            
            // 【修复】调换FlipX逻辑
            if (horizontalDirection < 0)
            {
                this->setFlippedX(false);  // 向左移动，面向左（FlipX=false）
            }
            else
            {
                this->setFlippedX(true);   // 向右移动，面向右（FlipX=true）
            }
        }
    }
    else
    {
        // 正常追击（包含垂直移动）
        // 归一化方向向量
        direction.normalize();
        
        // 应用速度
        Vec2 newPos = buzzerPos + direction * _chaseSpeed * dt;
        
        // 【新增】确保不会低于最低高度
        if (newPos.y < minHeight)
        {
            newPos.y = minHeight;
        }
        
        this->setPosition(newPos);
        
        // 【修复】调换FlipX逻辑
        if (direction.x < 0)
        {
            this->setFlippedX(false);  // 向左移动，面向左（FlipX=false）
        }
        else if (direction.x > 0)
        {
            this->setFlippedX(true);   // 向右移动，面向右（FlipX=true）
        }
    }
}

void Buzzer::changeState(State newState)
{
    if (_currentState == newState)
    {
        return;
    }

    _currentState = newState;

    switch (_currentState)
    {
    case State::IDLE:
        playIdleAnimation();
        break;

    case State::ATTACKING:
        playAttackAnimation();
        break;

    case State::DEAD:
        playDeathAnimation();
        break;
    }
}

void Buzzer::takeDamage(int damage, const cocos2d::Vec2& attackerPos)
{
    if (_currentState == State::DEAD || _isInvincible)
    {
        return;
    }

    _isInvincible = true;
    _health -= damage;
    CCLOG("?? Buzzer took %d damage! Health: %d/%d", damage, _health, _maxHealth);

    // ====== 新增：受击特效动画 ======
    float fxSize = std::max(this->getContentSize().width, this->getContentSize().height) * 0.8f;
    HitEffect::play(this->getParent(), this->getPosition() + Vec2(0, this->getContentSize().height * 0.5f), fxSize);
    // ===============================

    {
        changeState(State::DEAD);

        // ==========================================
        // 【新增】触发死亡回调 (加魂)
        // ==========================================
        if (_onDeathCallback)
        {
            _onDeathCallback();
            CCLOG("Buzzer died -> Trigger callback (Gain Soul)");
        }
    }
    // 【新增】受击击退效果
    Vec2 buzzerPos = this->getPosition();
    float directionX = buzzerPos.x - attackerPos.x;
    
    // 根据相对位置决定击退方向（远离攻击者）
    float knockbackDirection = (directionX > 0) ? 1.0f : -1.0f;
    
    // 击退距离和速度
    float knockbackDistance = 100.0f;
    float knockbackHeight = 50.0f;
    float knockbackDuration = 0.3f;
    
    Vec2 knockbackTarget = Vec2(
        buzzerPos.x + knockbackDirection * knockbackDistance,
        buzzerPos.y + knockbackHeight
    );
    
    // 应用击退动作
    this->stopActionByTag(777);
    auto jumpTo = JumpTo::create(knockbackDuration, knockbackTarget, knockbackHeight, 1);
    jumpTo->setTag(777);
    this->runAction(jumpTo);

    // 受击闪烁效果
    this->stopActionByTag(888);
    auto tintRed = TintTo::create(0.1f, 255, 0, 0);
    auto tintNormal = TintTo::create(0.1f, 255, 255, 255);
    auto blink = Sequence::create(tintRed, tintNormal, nullptr);
    auto repeat = Repeat::create(blink, 2);

    auto resetColor = CallFunc::create([this]() {
        this->setColor(Color3B::WHITE);
        this->setOpacity(255);
    });

    auto blinkSequence = Sequence::create(repeat, resetColor, nullptr);
    blinkSequence->setTag(888);
    this->runAction(blinkSequence);

    // 检查是否死亡
    if (_health <= 0)
    {
        CCLOG("? Buzzer defeated!");
        changeState(State::DEAD);
    }

    // 0.3秒后关闭无敌
    this->scheduleOnce([this](float dt) {
        _isInvincible = false;
    }, 0.3f, "invincible_cooldown");
}

void Buzzer::onCollideWithPlayer(const cocos2d::Vec2& playerPos)
{
    // 计算击退方向（远离玩家）
    Vec2 buzzerPos = this->getPosition();
    float directionX = buzzerPos.x - playerPos.x;
    
    // 根据相对位置决定击退方向
    float knockbackDirection = (directionX > 0) ? 1.0f : -1.0f;
    
    // 击退距离和速度
    float knockbackDistance = 80.0f;
    float knockbackHeight = 40.0f;
    float knockbackDuration = 0.25f;
    
    Vec2 knockbackTarget = Vec2(
        buzzerPos.x + knockbackDirection * knockbackDistance,
        buzzerPos.y + knockbackHeight
    );
    
    // 应用击退动作
    this->stopActionByTag(777);
    auto jumpTo = JumpTo::create(knockbackDuration, knockbackTarget, knockbackHeight, 1);
    jumpTo->setTag(777);
    this->runAction(jumpTo);
}

cocos2d::Rect Buzzer::getHitbox() const
{
    if (_currentState == State::DEAD)
    {
        return Rect::ZERO;
    }

    return this->getBoundingBox();
}

void Buzzer::setInitialPosition(const cocos2d::Vec2& pos)
{
    _initialPosition = pos;
    this->setPosition(pos);
}

Buzzer::~Buzzer()
{
    CC_SAFE_RELEASE(_idleAnimation);
    CC_SAFE_RELEASE(_attackAnimation);
}
