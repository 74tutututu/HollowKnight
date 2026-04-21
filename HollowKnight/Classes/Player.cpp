#include "Player.h"
#include "PlayerStates.h" // 引入状态类的实现
#include "config.h"   
#include "HelloWorldScene.h"
#include "Fireball.h" 
#include "HitEffect.h" // 引入受击特效

USING_NS_CC;

// =================================================================
//  1. 生命周期 (Lifecycle)
// =================================================================

Player::~Player()
{
    // 1. 释放组件内存
    CC_SAFE_DELETE(_stats);
    CC_SAFE_DELETE(_animator);

    this->stopAllActions();

    // 2. 停止所有定时器
    this->unscheduleAllCallbacks();
}

Player* Player::create(const std::string& filename)
{
    // 注意：这里为了架构整洁，忽略了 filename 参数，直接在 init 里加载 Config 指定的资源
    Player* player = new (std::nothrow) Player();
    if (player && player->init())
    {
        player->autorelease();
        return player;
    }
    CC_SAFE_DELETE(player);
    return nullptr;
}

bool Player::init()
{
    // 1. 加载初始纹理
    if (!this->initWithFile("Knight/idle/idle_1.png"))
    {
        CCLOG("Error: Failed to load 'Knight/idle/idle_1.png'");
        return false;
    }

    // 2. 设置锚点 (底边中心)
    this->setAnchorPoint(Vec2(0.5f, 0.0f));

    // 3. 初始化物理碰撞箱参数
    Size size = this->getContentSize();
    float bodyW = size.width * 0.6f;
    float bottomGap = 30.0f;
    float bodyH = size.height - bottomGap;

    _bodySize = Size(bodyW, bodyH);
    _bodyOffset = Vec2(0, bottomGap);
    _localBodyRect = Rect(-bodyW * 0.5f, bottomGap, bodyW, bodyH);

    // 4. 初始化变量
    // ==========================================
    // 【核心】初始化 Stats 组件 (保留组件化设计)
    // ==========================================
    _stats = new PlayerStats();
    // 依赖注入：把 Config 里的数值传给 Stats
    _stats->initStats(Config::DEFAULT_PLAYER_CFG);

    // 绑定回调：Stats 数据变了 -> 通知 Player -> Player 通知 UI
    _stats->onHealthChanged = [this](int cur, int max) {
        if (_onHealthChanged) _onHealthChanged(cur, max);
        };
    _stats->onSoulChanged = [this](int cur) {
        if (_onSoulChanged) _onSoulChanged(cur);
        };

    // 其他变量初始化
    _velocity = Vec2::ZERO;
    _isInvincible = false;
    _isFacingRight = false;
    _isOnGround = false;
    _attackCooldownTimer = 0.0f;
    _hasFireballSkill = false; // 初始不会

    // 初始化安全位置
    _lastSafePosition = this->getPosition();
    if (_lastSafePosition.equals(Vec2::ZERO)) {
        _lastSafePosition = Vec2(100, 500);
    }

    // 输入标记
    _inputDirectionX = 0;
    _inputDirectionY = 0;
    _isAttackPressed = false;
    _jumpTimer = 0.0f;
    _isJumpingAction = false;
    _isJumpPressed = false;
    _isFocusInputPressed = false;
    _jumpInputReleased = true; 
    _isCastPressed = false;
    _castInputReleased = true; 

    // 5. 加载所有动画资源 
    _animator = new PlayerAnimator();
    _animator->init(this); 

    // 6. 调试绘图
    _debugNode = DrawNode::create();
    this->addChild(_debugNode, 999);

    // 7. 启动状态机 - 进入待机状态
    this->changeState(new StateIdle());

    return true;
}

// =================================================================
//  2. 核心循环 (Core Loop)
// =================================================================

void Player::update(float dt, const std::vector<cocos2d::Rect>& platforms)
{
    // A. 攻击冷却倒计时
    if (_attackCooldownTimer > 0) {
        _attackCooldownTimer -= dt;
    }

    // B. 跳跃键重置检测
    // 只有当玩家【松开】跳跃键时，才允许下一次跳跃
    if (!_isJumpPressed) {
        _jumpInputReleased = true;
    }

    // 施法键重置检测
    if (!_isCastPressed) {
        _castInputReleased = true;
    }

    // 【逻辑层】委托给状态机处理
    if (_state)
    {
        _state->update(this, dt);
    }

    // 【物理层】执行位移和碰撞
    updateMovementX(dt);
    updateCollisionX(platforms);

    updateMovementY(dt);
    updateCollisionY(platforms);
}

// =================================================================
//  3. 状态机接口 (State Machine Interface)
// =================================================================

void Player::changeState(PlayerState* newState)
{
    if (_state)
    {
        _state->exit(this);
        delete _state;
    }

    _state = newState;

    if (_state)
    {
        _state->enter(this);
    }
}

// =================================================================
//  4. 动作执行接口 (Actions)
// =================================================================

void Player::moveInDirection(int dir)
{
    // 使用 Config::Player::MOVE_SPEED
    _velocity.x = dir * Config::Player::MOVE_SPEED;

    // 处理翻转
    if (dir != 0)
    {
        bool newFacingRight = (dir > 0);
        if (_isFacingRight != newFacingRight)
        {
            _isFacingRight = newFacingRight;
            this->setFlippedX(_isFacingRight);
        }
    }
}

float Player::getVelocityX()
{
    return _velocity.x;
}


void Player::setVelocityX(float x)
{
    _velocity.x = x;
}

void Player::setVelocityY(float y)
{
    _velocity.y = y;
}

void Player::startJump()
{
    _isOnGround = false;
    _isJumpingAction = true;
    _jumpTimer = 0.0f;
    _velocity.y = Config::Player::JUMP_FORCE_BASE;
}

void Player::stopJump()
{
    _isJumpingAction = false;

    // 截断跳跃 - 手感优化
    if (_velocity.y > 0)
    {
        _velocity.y *= 0.5f;
    }
}

void Player::attack()
{
    // 委托给 Animator 播放攻击特效
    _animator->playAttackEffect(_currentAttackDir, _isFacingRight);
}

void Player::startFocusEffect()
{
    _animator->startFocusEffect();
}

void Player::stopFocusEffect()
{
    _animator->stopFocusEffect();
}

void Player::playFocusEndEffect()
{
    _animator->playFocusEndEffect();
}

void Player::takeDamage(int damage, const cocos2d::Vec2& attackerPos, const std::vector<cocos2d::Rect>& platforms)
{
    // 1. 状态检查
    if (_isInvincible || _stats->isDead()) return;

    // 2. 扣血
    _stats->takeDamage(damage);
    CCLOG("Player took damage! Health: %d", _stats->getHealth());

    // 3. 受击特效（适配主角大小，居中）
    float fxSize = std::max(this->getContentSize().width, this->getContentSize().height) * 0.8f;
    HitEffect::play(this->getParent(), this->getPosition() + Vec2(0, this->getContentSize().height * 0.5f), fxSize);

    // 4. 计算正确的击退方向 (远离攻击者)
    float knockbackSpeed = 400.0f;
    float direction = (this->getPositionX() < attackerPos.x) ? -1.0f : 1.0f;

    // 5. UI 更新
    if (_onHealthChanged) {
        _onHealthChanged(this->getHealth(), this->getMaxHealth());
    }

    _velocity.x = direction * knockbackSpeed;
    _velocity.y = 300.0f; // 给一个小跳，防止在地面摩擦力过大

    // 6. 切换状态和无敌
    changeState(new StateDamaged());

    _isInvincible = true;
    auto blink = RepeatForever::create(Sequence::create(
        FadeTo::create(0.1f, 100),
        FadeTo::create(0.1f, 255),
        nullptr
    ));
    blink->setTag(999);
    this->runAction(blink);

    this->scheduleOnce([this](float dt) {
        _isInvincible = false;
        this->stopActionByTag(999);
        this->setOpacity(255);
        }, 1.0f, "invincible_end");
}

void Player::executeHeal()
{
    // 调用组件尝试回血
    bool success = _stats->recoverHealth();
}

void Player::pogoJump()
{
    _velocity.y = 0;
    // 给一个向上的瞬时速度 (类似跳跃)
    _velocity.y = Config::Player::JUMP_FORCE_BASE * 1.2f;
    _isOnGround = false;
}

// 实现消耗灵魂
bool Player::consumeSoul(int amount)
{
    if (!_stats) return false;
    if (getSoul() >= amount) {
        gainSoul(-amount); // 扣除灵魂
        return true;
    }
    return false;
}

bool Player::canCastSpell()
{
    // 1. 没解锁不能放
    if (!_hasFireballSkill) return false;

    if (getSoul() < Config::Skill::FIREBALL_COST) return false;

    return true;
}

void Player::executeSpell()
{
    // 1. 扣蓝
    consumeSoul(Config::Skill::FIREBALL_COST);

    // 2. 生成火球
    auto fireball = Fireball::create("fireball/idle/fireball_1.png");
    if (fireball)
    {
        // 计算位置：在主角前方 50 像素，高度微调
        float dir = _isFacingRight? 1.0f : -1.0f;
        Vec2 spawnPos = this->getPosition() + Vec2(dir * 50.0f, 80.0f);

        fireball->setPosition(spawnPos);

        // B. 设置方向和速度
        fireball->shoot(Config::Skill::FIREBALL_SPEED, (int)dir);
        fireball->setTag(5000); // 统一 Tag

        // C. 添加到场景 (核心技巧：Player 的父节点就是 GameLayer)
        if (this->getParent()) {
            this->getParent()->addChild(fireball, 10);
        }

        // E. 给主角一个反冲力 (Hollow Knight 细节：施法会有后坐力)
        this->setVelocityX(dir * -200.0f);
    }
}

// =================================================================
//  5. 动画系统
// =================================================================

void Player::playAnimation(const std::string& animName)
{
    _animator->playAnimation(animName);
}

// =================================================================
//  6. 输入设置
// =================================================================
void Player::setInputDirectionX(int dir) { _inputDirectionX = dir; }
void Player::setInputDirectionY(int dir) { _inputDirectionY = dir; }
void Player::setAttackPressed(bool pressed) { _isAttackPressed = pressed; }
void Player::setJumpPressed(bool pressed) { _isJumpPressed = pressed; }
void Player::setAttackDir(int dir) { _currentAttackDir = dir; }
void Player::setFocusInput(bool pressed) { _isFocusInputPressed = pressed; }
void Player::setCastInput(bool pressed){    _isCastPressed = pressed; }
void Player::setDreamNailInput(bool pressed){   _isDreamNailPressed = pressed; }
// =================================================================
//  7. 物理引擎 (使用 Config)
// =================================================================

void Player::updateMovementX(float dt)
{
    float dx = _velocity.x * dt;
    this->setPositionX(this->getPositionX() + dx);
}

void Player::updateMovementY(float dt)
{
    // 长按跳跃增高
    if (_isJumpingAction)
    {
        _jumpTimer += dt;
        // 使用 Config::Player::MAX_JUMP_TIME
        if (_jumpTimer < Config::Player::MAX_JUMP_TIME) {
            // 使用 Config::Player::JUMP_ACCEL
            _velocity.y += Config::Player::JUMP_ACCEL * dt;
        }
        else {
            _isJumpingAction = false;
        }
    }

    // 重力: 使用 Config::Player::GRAVITY
    _velocity.y -= Config::Player::GRAVITY * dt;

    // 终端速度: 使用 Config::Player::MAX_FALL_SPEED
    if (_velocity.y < Config::Player::MAX_FALL_SPEED)
        _velocity.y = Config::Player::MAX_FALL_SPEED;

    float dy = _velocity.y * dt;
    this->setPositionY(this->getPositionY() + dy);

    // ============================================================
    // 防Bug检测：如果掉出地图，拉回
    // ============================================================
    if (this->getPositionY() < -50.0f)
    {
        CCLOG("[Player] BUG DETECTED: Fell out of map! Teleporting to safety.");

        // 1. 瞬移回上一次的安全地板 (稍微抬高 50 像素，防止再次卡进地里)
        // 如果 _lastSafePosition 还没初始化(0,0)，就用当前 X 坐标 + 高度兜底
        if (_lastSafePosition.equals(Vec2::ZERO)) {
            this->setPosition(this->getPositionX(), 1000.0f);
        }
        else {
            this->setPosition(_lastSafePosition + Vec2(0, 50.0f));
        }

        // 2. 彻底锁死速度 (防止带着下坠速度再次掉下去)
        _velocity = Vec2::ZERO;

        // 3. 如果在跳跃状态，强制停止，防止逻辑混乱
        _isJumpingAction = false;
    }
}

void Player::updateCollisionX(const std::vector<cocos2d::Rect>& platforms)
{
    Rect playerRect = getCollisionBox();
    for (const auto& wall : platforms)
    {
        if (playerRect.intersectsRect(wall))
        {
            float overlapY = std::min(playerRect.getMaxY(), wall.getMaxY()) -
                std::max(playerRect.getMinY(), wall.getMinY());

            if (overlapY > playerRect.size.height * 0.5f)
            {
                if (_velocity.x > 0)
                {
                    this->setPositionX(wall.getMinX() - _bodySize.width * 0.5f - _bodyOffset.x - 0.1f);
                    _velocity.x = 0;
                }
                else if (_velocity.x < 0)
                {
                    this->setPositionX(wall.getMaxX() + _bodySize.width * 0.5f - _bodyOffset.x + 0.1f);
                    _velocity.x = 0;
                }
            }
        }
    }
}

void Player::updateCollisionY(const std::vector<cocos2d::Rect>& platforms)
{
    _isOnGround = false;
    Rect playerRect = getCollisionBox();

    for (const auto& platform : platforms)
    {
        if (playerRect.intersectsRect(platform))
        {
            float overlapX = std::min(playerRect.getMaxX(), platform.getMaxX()) -
                std::max(playerRect.getMinX(), platform.getMinX());

            if (overlapX > playerRect.size.width * 0.1f)
            {
                if (_velocity.y <= 0)
                {
                    float tolerance = 40.0f;
                    float overlapY = platform.getMaxY() - playerRect.getMinY();

                    if (overlapY > -0.1f && overlapY <= tolerance)
                    {
                        this->setPositionY(platform.getMaxY() - _bodyOffset.y - 1.0f);
                        _velocity.y = 0;
                        _isOnGround = true;

                        // ==========================================
                        // 记录安全坐标
                        // ==========================================
                        _lastSafePosition = this->getPosition();
                    }
                }
                else if (_velocity.y > 0)
                {
                    float tolerance = 20.0f;
                    float overlapY = playerRect.getMaxY() - platform.getMinY();
                    if (overlapY > 0 && overlapY <= tolerance)
                    {
                        this->setPositionY(platform.getMinY() - _bodySize.height - _bodyOffset.y);
                        _velocity.y = 0;
                    }
                }
            }
        }
    }
}

// =================================================================
//  8. 辅助函数
// =================================================================
void Player::startAttackCooldown()
{
    _attackCooldownTimer = 0.25f+6*Config::Player::ATTACK_COOLDOWN;
}

bool Player::isAttackReady() const
{
    return _attackCooldownTimer <= 0.0f;
}

bool Player::isJumpReady() const
{
    return _jumpInputReleased;
}

void Player::consumeJumpInput()
{
    // 锁死跳跃，直到下次松开按键
    _jumpInputReleased = false;
}

bool Player::isCastReady() const
{
    return _castInputReleased;
}

void Player::consumeCastInput()
{
    _castInputReleased = false; // 锁死，直到下次松手
}

cocos2d::Rect Player::getCollisionBox() const
{
    Vec2 worldPos = this->getPosition();
    return Rect(
        worldPos.x + _localBodyRect.origin.x,
        worldPos.y + _localBodyRect.origin.y,
        _localBodyRect.size.width,
        _localBodyRect.size.height
    );
}

cocos2d::Rect Player::getAttackHitbox() const
{
    Vec2 pos = this->getPosition();

    // ================== 上劈判定 ==================
    if (_currentAttackDir == 1)
    {
        float w = 100.0f;
        float h = 130.0f;
        float startX = pos.x - w / 2 + _bodyOffset.x;
        // Y轴：从头顶往下一点点开始，向上延伸
        float startY = pos.y + _bodySize.height + _bodyOffset.y - 30.0f;
        return Rect(startX, startY, w, h);
    }

    // ================== 下劈判定 ==================
    else if (_currentAttackDir == -1)
    {
        float w = 100.0f;
        float h = 120.0f;
        // X轴：居中
        float startX = pos.x - w / 2 + _bodyOffset.x;

        // Y轴：让判定框从“脚踝以上”就开始，向下延伸
        float startY = pos.y + _bodyOffset.y - h + 40.0f;

        return Rect(startX, startY, w, h);
    }

    // ================== 水平判定 ==================
    else {
        // 【合并点】采用文件2调优后的参数 (140.0f)
        float attackRange = 140.0f;
        float attackHeight = 70.0f;
        float innerOffset = 20.0f;

        float startY = pos.y + _bodyOffset.y + 10.0f;
        float startX = _isFacingRight ? (pos.x - innerOffset) : (pos.x + innerOffset - attackRange);

        return Rect(startX, startY, attackRange, attackHeight);
    }
}

cocos2d::Rect Player::getDreamNailHitbox() const
{
    Vec2 pos = this->getPosition();
    float range = Config::Player::DREAM_NAIL_RANGE;
    float h = 80.0f;
    float startX = _isFacingRight ? pos.x : (pos.x - range);
    return Rect(startX, pos.y + 20, range, h);
}

void Player::drawDebugRects()
{
    if (!_debugNode) return;
    _debugNode->clear();

    Size size = this->getContentSize();
    Vec2 centerOffset = Vec2(size.width * 0.5f, 0.0f);

    // A. 画物理框 (实心绿)
    Vec2 greenMin = _localBodyRect.origin + centerOffset;
    Vec2 greenMax = greenMin + _localBodyRect.size;
    _debugNode->drawSolidRect(greenMin, greenMax, Color4F(0, 1, 0, 0.4f));

    // B. 画图片轮廓 (空心蓝)
    _debugNode->drawRect(Vec2::ZERO, Vec2(size.width, size.height), Color4F(0, 0, 1, 0.5f));

    // C. 画锚点位置 (品红点)
    _debugNode->drawDot(centerOffset, 5.0f, Color4F::MAGENTA);

    // D. 画攻击判定框 (红色)
    // 简单绘制一个示意框，实际逻辑在 getAttackHitbox 里
    // 这里为了调试准确，其实可以直接画 getAttackHitbox() 的返回结果
    // 但因为 DrawNode 是局部坐标，getAttackHitbox 是世界坐标，需要转换
    // 这里保留原有的简单绘制
    if (_debugNode)
    {
        float attackRange = 140.0f; // 保持一致
        float attackHeight = 70.0f;
        float innerOffset = 20.0f;
        float startY = 0 + _bodyOffset.y + 10.0f;
        float startX;

        if (_isFacingRight) {
            startX = (size.width / 2) - innerOffset;
        }
        else {
            startX = (size.width / 2) + innerOffset - attackRange;
        }

        Vec2 origin(startX, startY);
        Vec2 dest(startX + attackRange, startY + attackHeight);
        _debugNode->drawRect(origin, dest, Color4F(1, 0, 0, 0.6f));
    }
}

bool Player::canFocus() const
{
    // 安全检查，防止 stats 还没初始化就调用崩溃
    if (!_stats) return false;
    return _stats->canFocus();
}

// 2. 修复 gainSoul (转发给组件)
void Player::gainSoul(int amount)
{
    if (_stats) {
        _stats->gainSoul(amount);
    }
}

void Player::gainSoulOnKill()
{
    if (_stats) {
        _stats->gainSoulOnKill();
    }
}

// 3. 修复 UI 回调绑定
void Player::setOnHealthChanged(const std::function<void(int, int)>& callback)
{
    _onHealthChanged = callback; // 保存到 Player 自身
    if (_stats) {
        _stats->onHealthChanged = callback; // 同时设置给 Stats 组件
    }
}

void Player::setOnSoulChanged(const std::function<void(int)>& callback)
{
    _onSoulChanged = callback;
    if (_stats) {
        _stats->onSoulChanged = callback;
    }
}

// 新增：施法后刷新地面判定和安全坐标
void Player::recordSafePositionIfOnGround()
{
    if (_isOnGround) {
        _lastSafePosition = this->getPosition();
    }
}