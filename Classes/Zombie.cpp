#include "Zombie.h"
#include "HitEffect.h"

USING_NS_CC;

Zombie* Zombie::create(const std::string& filename)
{
    Zombie* zombie = new (std::nothrow) Zombie();
    if (zombie && zombie->initWithFile(filename) && zombie->init())
    {
        zombie->autorelease();
        CCLOG("? [Zombie::create] Succeeded: %s", filename.c_str());
        return zombie;
    }
    CC_SAFE_DELETE(zombie);
    return nullptr;
}

bool Zombie::init()
{
    if (!Sprite::init()) return false;

    // 初始化状态与属性
    _currentState = State::PATROL;
    _health = 5;
    _maxHealth = 5;
    _moveSpeed = 40.0f;
    _attackSpeed = 200.0f;
    _movingRight = true;
    _isFacingRight = true;
    _isInvincible = false;

    // AI 参数
    _patrolLeftBound = 0.0f;
    _patrolRightBound = 300.0f;
    _detectionRange = 400.0f;

    // 追逐范围限制 (全屏宽度)
    auto visibleSize = Director::getInstance()->getVisibleSize();
    _maxChaseRange = visibleSize.width * 3.0f;
    _spawnPosition = Vec2::ZERO; // 稍后更新

    // 物理参数初始化
    _velocity = Vec2::ZERO;
    _isOnGround = false;
    _gravity = 2000.0f;
    _maxFallSpeed = -1500.0f;

    // 设置梦之钉心声 (GameEntity 提供的功能)
    this->setDreamThought("...Brains...Hungry...");

    loadAnimations();
    playWalkAnimation();

    return true;
}

void Zombie::loadAnimations()
{
    // --- 走路动画 ---
    Vector<SpriteFrame*> walkFrames;
    for (int i = 1; i <= 7; i++) {
        auto sprite = Sprite::create(StringUtils::format("zombie/walk/walk_%d.png", i));
        if (sprite) walkFrames.pushBack(sprite->getSpriteFrame());
    }
    if (!walkFrames.empty()) {
        _walkAnimation = Animation::createWithSpriteFrames(walkFrames, 0.15f);
        _walkAnimation->retain();
    }

    // --- 准备攻击动画 ---
    Vector<SpriteFrame*> readyFrames;
    for (int i = 1; i <= 5; i++) {
        auto sprite = Sprite::create(StringUtils::format("zombie/attack/attackReady_%d.png", i));
        if (sprite) readyFrames.pushBack(sprite->getSpriteFrame());
    }
    if (!readyFrames.empty()) {
        _attackReadyAnimation = Animation::createWithSpriteFrames(readyFrames, 0.1f);
        _attackReadyAnimation->retain();
    }

    // --- 攻击动画 ---
    Vector<SpriteFrame*> attackFrames;
    for (int i = 1; i <= 3; i++) {
        auto sprite = Sprite::create(StringUtils::format("zombie/attack/attack_%d.png", i));
        if (sprite) attackFrames.pushBack(sprite->getSpriteFrame());
    }
    if (!attackFrames.empty()) {
        _attackAnimation = Animation::createWithSpriteFrames(attackFrames, 0.1f);
        _attackAnimation->retain();
    }
}

// ========================================
// 动画播放函数 (带状态保护)
// ========================================
void Zombie::playWalkAnimation() {
    if (_walkAnimation) {
        this->stopActionByTag(100);
        auto act = RepeatForever::create(Animate::create(_walkAnimation));
        act->setTag(100);
        this->runAction(act);
    }
}

void Zombie::playAttackReadyAnimation() {
    if (_attackReadyAnimation) {
        this->stopActionByTag(100);
        auto animate = Animate::create(_attackReadyAnimation);
        animate->setTag(100);
        this->runAction(animate);

        // 动画播完自动进入攻击状态
        this->scheduleOnce([this](float) {
            changeState(State::ATTACKING);
            }, _attackReadyAnimation->getDuration(), "ready_end");
    }
}

void Zombie::playAttackAnimation() {
    if (_attackAnimation) {
        this->stopActionByTag(100);
        auto act = RepeatForever::create(Animate::create(_attackAnimation));
        act->setTag(100);
        this->runAction(act);
    }
}

void Zombie::playDeathAnimation() {
    this->stopAllActions(); // 死亡停止一切
    auto seq = Sequence::create(
        Spawn::create(FadeOut::create(0.5f), RotateBy::create(0.5f, 180), nullptr),
        CallFunc::create([this]() {
            this->removeFromParent();
            CCLOG("Zombie Dead & Removed");
            }),
        nullptr
    );
    this->runAction(seq);
}

// ========================================
// 核心 Update 
// ========================================
void Zombie::update(float dt, const cocos2d::Vec2& playerPos, const std::vector<cocos2d::Rect>& platforms)
{
    if (_currentState == State::DEAD) return;

    // 1. 初始化出生点
    if (_spawnPosition == Vec2::ZERO) _spawnPosition = this->getPosition();

    // ============================================================
    // 【修改：掉落保护】
    // 如果掉出地图 (穿模了)，不要删除，而是“拉回”出生点
    // ============================================================
    if (this->getPositionY() < -300.0f) {
        CCLOG("[Zombie] Fell off map! Teleporting back to spawn.");

        // 1. 重置位置到出生点 (稍微抬高一点防止卡地)
        this->setPosition(_spawnPosition + Vec2(0, 50));

        // 2. 物理速度清零 (防止带着巨大的下落速度再次穿模)
        _velocity = Vec2::ZERO;

        // 3. 重置状态为巡逻
        changeState(State::PATROL);
        return;
    }

    // ===================================
    // 2. 物理系统：Y轴 (重力与地面)
    // ===================================
    updateMovementY(dt);
    updateCollisionY(platforms);

    // 受伤状态下只应用物理，不思考
    if (_currentState == State::DAMAGED) {
        updateMovementX(dt);
        updateCollisionX(platforms);
        return;
    }

    // ===================================
    // 3. AI 决策系统
    // ===================================
    switch (_currentState)
    {
    case State::PATROL:
        if (isPlayerInRange(playerPos)) {
            // 发现玩家 -> 转向并准备攻击
            float dir = playerPos.x - this->getPositionX();
            _isFacingRight = (dir > 0);
            this->setFlippedX(!_isFacingRight);
            changeState(State::ATTACK_READY);
        }
        else {
            updatePatrolBehavior(dt);
        }
        break;

    case State::ATTACK_READY:
        _velocity.x = 0; // 准备时不动
        break;

    case State::ATTACKING:
        // 检查追逐范围
        if (!isPlayerInChaseRange(playerPos)) {
            CCLOG("[Zombie] Player too far, give up.");
            changeState(State::PATROL);
        }
        else {
            updateAttackBehavior(dt, playerPos);
        }
        break;

    default: break;
    }

    // ===================================
    // 4. 物理系统：X轴 (移动与墙壁)
    // ===================================
    // 只有非静止状态才需要更新X物理
    if (_velocity.x != 0) {
        updateMovementX(dt);
        updateCollisionX(platforms);
    }
}

// ========================================
// AI 行为逻辑
// ========================================
bool Zombie::isPlayerInRange(const cocos2d::Vec2& playerPos) {
    return std::abs(playerPos.x - this->getPositionX()) <= _detectionRange;
}

bool Zombie::isPlayerInChaseRange(const cocos2d::Vec2& playerPos) {
    return std::abs(playerPos.x - _spawnPosition.x) <= _maxChaseRange;
}

void Zombie::updatePatrolBehavior(float dt) {
    if (_movingRight) {
        _velocity.x = _moveSpeed;
        if (getPositionX() >= _patrolRightBound) {
            _movingRight = false;
            _isFacingRight = false;
            this->setFlippedX(true);
        }
    }
    else {
        _velocity.x = -_moveSpeed;
        if (getPositionX() <= _patrolLeftBound) {
            _movingRight = true;
            _isFacingRight = true;
            this->setFlippedX(false);
        }
    }
}

void Zombie::updateAttackBehavior(float dt, const cocos2d::Vec2& playerPos) {
    float dir = playerPos.x - getPositionX();

    // 简单的追逐逻辑
    if (_isFacingRight && dir > 0) _velocity.x = _attackSpeed;
    else if (!_isFacingRight && dir < 0) _velocity.x = -_attackSpeed;
    else {
        // 玩家跑到身后了 -> 放弃攻击，继续巡逻
        changeState(State::PATROL);
    }
}

// ========================================
// 受击与物理反馈
// ========================================
void Zombie::takeDamage(int damage, const cocos2d::Vec2& attackerPos)
{
    if (_currentState == State::DEAD || _currentState == State::DAMAGED || _isInvincible) return;

    _health -= damage;
    _isInvincible = true;
    CCLOG("Zombie Hit! HP: %d", _health);

    // ====== 新增：受击特效动画（位置略微偏下） ======
    float fxSize = std::max(this->getContentSize().width, this->getContentSize().height) * 0.8f;
    HitEffect::play(this->getParent(), this->getPosition() + Vec2(0, this->getContentSize().height * 0.15f), fxSize);
    // ===============================

    // 1. 死亡判断
    if (_health <= 0) {
        changeState(State::DEAD);
        if (_onDeathCallback) _onDeathCallback();
        return;
    }

    State lastState = _currentState;
    changeState(State::DAMAGED);

    // 2. 受击击退
    float dir = (getPositionX() - attackerPos.x) > 0 ? 1.0f : -1.0f;
    _velocity.x = dir * 300.0f;
    _velocity.y = 200.0f;

    // 3. 闪烁动画
    this->stopActionByTag(888);
    auto blink = Sequence::create(
        Repeat::create(Sequence::create(TintTo::create(0.1f, 255, 0, 0), TintTo::create(0.1f, 255, 255, 255), nullptr), 2),
        CallFunc::create([this]() { this->setColor(Color3B::WHITE); this->setOpacity(255); }),
        nullptr
    );
    blink->setTag(888);
    this->runAction(blink);

    // 4. 恢复状态
    this->scheduleOnce([this, lastState](float) {
        if (_currentState == State::DAMAGED) {
            _velocity.x = 0;
            changeState(State::PATROL);
        }
        }, 0.3f, "recover_state");

    // 5. 恢复无敌
    this->scheduleOnce([this](float) { _isInvincible = false; }, 0.2f, "recover_invincible");
}

void Zombie::onCollideWithPlayer(const cocos2d::Vec2& playerPos)
{
    if (_currentState == State::DEAD || _currentState == State::DAMAGED) return;

    // 简单的物理反弹
    float dir = (getPositionX() - playerPos.x) > 0 ? 1.0f : -1.0f;
    _velocity.x = dir * 150.0f; // 弹开一点点

    // 0.2秒后摩擦力停下
    this->scheduleOnce([this](float) {
        if (_currentState != State::DAMAGED) _velocity.x = 0;
        }, 0.2f, "stop_bounce");
}

void Zombie::changeState(State newState)
{
    if (_currentState == newState) return;
    _currentState = newState;

    switch (_currentState) {
    case State::PATROL: playWalkAnimation(); break;
    case State::ATTACK_READY: playAttackReadyAnimation(); break;
    case State::ATTACKING: playAttackAnimation(); break;
    case State::DEAD: playDeathAnimation(); break;
    case State::DAMAGED: break; // 保持动作
    }
}

void Zombie::setPatrolRange(float left, float right) {
    _patrolLeftBound = left;
    _patrolRightBound = right;
}

cocos2d::Rect Zombie::getHitbox() const {
    if (_currentState == State::DEAD) return Rect::ZERO;
    // 稍微降低高度，更加贴合视觉
    Rect box = this->getBoundingBox();
    box.size.height *= 0.7f;
    return box;
}

// ========================================
// 物理引擎核心 (来自 File 1)
// ========================================
void Zombie::updateMovementY(float dt) {
    _velocity.y -= _gravity * dt;
    if (_velocity.y < _maxFallSpeed) _velocity.y = _maxFallSpeed;
    setPositionY(getPositionY() + _velocity.y * dt);
}

void Zombie::updateCollisionY(const std::vector<cocos2d::Rect>& platforms) {
    _isOnGround = false;
    Rect rect = this->getBoundingBox();
    for (const auto& wall : platforms) {
        if (rect.intersectsRect(wall)) {
            float overlapX = std::min(rect.getMaxX(), wall.getMaxX()) - std::max(rect.getMinX(), wall.getMinX());
            if (overlapX > rect.size.width * 0.3f) {
                if (_velocity.y <= 0) { // 下落
                    float overlapY = wall.getMaxY() - rect.getMinY();
                    if (overlapY > -0.1f && overlapY <= 40.0f) {
                        setPositionY(wall.getMaxY() + rect.size.height * 0.5f);
                        _velocity.y = 0;
                        _isOnGround = true;
                    }
                }
            }
        }
    }
}

void Zombie::updateMovementX(float dt) {
    setPositionX(getPositionX() + _velocity.x * dt);
}

void Zombie::updateCollisionX(const std::vector<cocos2d::Rect>& platforms) {
    Rect rect = this->getBoundingBox();
    for (const auto& wall : platforms) {
        if (rect.intersectsRect(wall)) {
            float overlapY = std::min(rect.getMaxY(), wall.getMaxY()) - std::max(rect.getMinY(), wall.getMinY());
            if (overlapY > rect.size.height * 0.5f) {
                if (_velocity.x > 0) { // 向右撞
                    setPositionX(wall.getMinX() - rect.size.width * 0.5f - 0.1f);
                    _velocity.x = 0;
                    if (_currentState == State::PATROL) { // 自动转向
                        _movingRight = false;
                        _isFacingRight = false;
                        this->setFlippedX(true);
                    }
                }
                else if (_velocity.x < 0) { // 向左撞
                    setPositionX(wall.getMaxX() + rect.size.width * 0.5f + 0.1f);
                    _velocity.x = 0;
                    if (_currentState == State::PATROL) {
                        _movingRight = true;
                        _isFacingRight = true;
                        this->setFlippedX(false);
                    }
                }
            }
        }
    }
}

Zombie::~Zombie() {
    CC_SAFE_RELEASE(_walkAnimation);
    CC_SAFE_RELEASE(_attackReadyAnimation);
    CC_SAFE_RELEASE(_attackAnimation);
    CC_SAFE_RELEASE(_deathAnimation);
}