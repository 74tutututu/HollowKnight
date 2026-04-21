#include "Boss.h"
#include "FKFireball.h" // 【新增】
#include "HitEffect.h"

USING_NS_CC;

// ============================================================
// 常量配置
// ============================================================
const float GRAVITY = -1400.0f;

// 普通跳跃：增加中空高度 (1050 -> 1200)
const float JUMP_FORCE_NORMAL = 1200.0f;

// 攻击跳跃：增加力度 (1120 -> 1350)
// 配合 0.6s 的中空动画，整体中空时间约 1.93秒
const float JUMP_FORCE_ATTACK = 1350.0f;

// 水平速度限制
const float MIN_JUMP_SPEED_X = 100.0f;
const float MAX_JUMP_SPEED_X = 500.0f;
const float MAX_ATTACK_SPEED_X = 50.0f; // 攻击时几乎原地起跳

const float BOSS_MIN_X = 470.0f;  // 场地左右限制，减少穿模
const float BOSS_MAX_X = 2558.0f;

// 瘫痪相关
const int MAX_HITS_BEFORE_STUN = 13;
const int MAX_STUN_HITS = 6;
const int STUN_HP_TOTAL = 18; // 仅在 Stunned 状态下扣血，总计18点（平A=1，法术=2）
const float STUN_DURATION = 10.0f;
const float HURT_COOLDOWN = 0.2f;

// 缩放比例
const float BOSS_SCALE = 1.4f;

// Action Tags
const int TAG_ANIMATION = 100;
const int TAG_FLASH = 200;
const int TAG_DEBUG_DRAW = 999; // 调试绘图节点的Tag
const int TAG_HAMMER_WINDOW = 250; // Hammer 判定窗口

// 【新增】狂暴模式常量
const float RAMPAGE_JUMP_FORCE = 1300.0f;
const float RAMPAGE_FIREBALL_INTERVAL = 0.2f; // 火球生成间隔
const int RAMPAGE_ATTACK_COUNT = 6; // 6轮攻击

const float SHOCKWAVE_FRAME_DELAY = 0.1f;
const float SHOCKWAVE_SPAWN_OFFSET_X = 160.0f; // 以Boss面向方向偏移
const float SHOCKWAVE_SPAWN_OFFSET_Y = 40.0f;

// ============================================================
// 实现部分
// ============================================================

Boss* Boss::create(const Vec2& spawnPos)
{
    Boss* pRet = new(std::nothrow) Boss();
    if (pRet && pRet->init(spawnPos))
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

bool Boss::init(const Vec2& spawnPos)
{
    if (!Node::init()) return false;

    this->setPosition(spawnPos);
    this->setName("Normal"); // 默认为普通状态
    _velocity = Vec2(0, 0);

    // 初始化数值
    _hitCount = 0;
    _stunHitCount = 0;
    _stunHP = STUN_HP_TOTAL;
    _isDead = false;
    _onGround = false;
    _actionStep = 0;
    _stateTimer = 0.0f;
    _pendingIdleCount = 0;
    _hurtTimer = 0.0f;
    _isStunAnimPlaying = false;
    _isAttackLanded = false;
    _isHammerActive = false; // 初始化伤害标记
    _lastPlayerX = 0.0f;
    _facing = 1.0f; // 1.0f for right, -1.0f for left
    _rampageCounter = 0;
    _rampageFireballTimer = 0.0f;
    _isRampaging = false;

    // 创建精灵
    _sprite = Sprite::create("boss/fall/1.png");
    if (_sprite) {
        _sprite->setAnchorPoint(Vec2(0.5f, 0.0f));
        _sprite->setScale(BOSS_SCALE);
        this->addChild(_sprite);
    }

    else {
        // 【关键】如果图片没找到，打印错误并停止初始化！
        // 这能防止后续逻辑操作空数据导致的崩溃
        CCLOG("ERROR: Failed to load Boss sprite 'boss/fall/1.png'! Crash avoided.");
        return false;
    }

    // 【调试】增加绘图节点，用于显示碰撞体积
    // auto drawNode = DrawNode::create();
    // drawNode->setTag(TAG_DEBUG_DRAW);
    // this->addChild(drawNode, 999); // 放在最上层

    switchState(State::Falling_Enter);

    return true;
}

void Boss::updateBoss(float dt, const Vec2& playerPos, const std::vector<Rect>& groundRects)
{
    if (_isDead) return;

    // 记录主角位置
    _lastPlayerX = playerPos.x;

    // 0. 更新受击冷却
    if (_hurtTimer > 0) {
        _hurtTimer -= dt;
    }

    // 1. 面向主角
    if (_state != State::Stunned && _state != State::Recovering &&
        _state != State::Jumping && _state != State::Jump_Attack &&
        _state != State::Rampage_Jump && _state != State::Rampage_Attack &&
        _state != State::Shockwave_Attack) {
        setFacing(playerPos.x);
    }

    // 2. 物理模拟
    _velocity.y += GRAVITY * dt;

    // 3. 状态机逻辑更新
    if (_state == State::Idle)
    {
        _stateTimer -= dt;
        if (_stateTimer <= 0) {
            performNextAction();
        }
    }
    else if (_state == State::Stunned)
    {
        if (!_isStunAnimPlaying) {
            _stunTimer -= dt;
            if (_stunTimer <= 0) {
                CCLOG("Boss Stun Time Up! Recovering...");
                recoverFromStun();
            }
        }
    }
    else if (_isRampaging) // 【新增】狂暴状态持续生成火球
    {
        _rampageFireballTimer -= dt;
        while (_rampageFireballTimer <= 0)
        {
            if (_fireballCallback)
            {
                float randomX = cocos2d::random(470.0f, 2558.0f);
                float y = this->getPositionY() + 800.0f;
                _fireballCallback(Vec2(randomX, y));
            }
            _rampageFireballTimer += RAMPAGE_FIREBALL_INTERVAL;
        }
    }

    // 4. 应用位移
    Vec2 currentPos = this->getPosition();
    Vec2 nextPos = currentPos + _velocity * dt;

    // 5. 地面碰撞检测
    _onGround = false;
    Rect bodyRect = this->getBodyHitbox();
    bodyRect.origin = nextPos - Vec2(bodyRect.size.width / 2, 0);

    for (const auto& rect : groundRects)
    {
        if (currentPos.y >= rect.getMaxY() && nextPos.y <= rect.getMaxY())
        {
            if (currentPos.x >= rect.getMinX() - 50 && currentPos.x <= rect.getMaxX() + 50)
            {
                nextPos.y = rect.getMaxY();
                _velocity.y = 0;
                _velocity.x = 0;
                _onGround = true;
                onLand();
                break;
            }
        }
    }

    if (nextPos.x < 0) nextPos.x = 0;
    nextPos.x = std::max(BOSS_MIN_X, std::min(BOSS_MAX_X, nextPos.x));

    this->setPosition(nextPos);

    // 可视化辅助线已移除
    // auto drawNode = static_cast<DrawNode*>(this->getChildByTag(TAG_DEBUG_DRAW));
    // if (drawNode) {
    //     drawNode->clear();
    //
    //     // 1. Boss身体碰撞体
    //     Rect worldBody = getBodyHitbox();
    //     Vec2 offset = this->getPosition();
    //     Rect localBody(worldBody.origin.x - offset.x, worldBody.origin.y - offset.y, worldBody.size.width, worldBody.size.height);
    //     drawNode->drawSolidRect(localBody.origin, localBody.origin + localBody.size, Color4F(1, 0, 0, 0.3f));
    //     drawNode->drawRect(localBody.origin, localBody.origin + localBody.size, Color4F::RED);
    //
    //     // 2. 锤子碰撞体
    //     Rect worldHammer = getHammerHitbox();
    //     if (!worldHammer.equals(Rect::ZERO)) {
    //         Rect localHammer(worldHammer.origin.x - offset.x, worldHammer.origin.y - offset.y, worldHammer.size.width, worldHammer.size.height);
    //         drawNode->drawSolidRect(localHammer.origin, localHammer.origin + localHammer.size, Color4F(1, 1, 0, 0.3f));
    //         drawNode->drawRect(localHammer.origin, localHammer.origin + localHammer.size, Color4F::YELLOW);
    //     }
    // }
}

void Boss::performNextAction()
{
    if (_pendingIdleCount > 0)
    {
        _pendingIdleCount--;
        switchState(State::Idle);
        return;
    }

    int choice = cocos2d::random(0, 2); // 0: jump, 1: jump attack, 2: shockwave
    if (choice == 0)
    {
        switchState(State::Pre_Jump);
    }
    else if (choice == 1)
    {
        switchState(State::Pre_Attack);
    }
    else
    {
        switchState(State::Shockwave_Attack);
    }

    _pendingIdleCount = 2; // 每个招式后强制两轮Idle
}

void Boss::switchState(State newState)
{
    _state = newState;

    // 状态切换时，无条件强制关闭锤子伤害
    _isHammerActive = false;
    this->stopActionByTag(TAG_HAMMER_WINDOW);

    // 状态切换时，先重置名称为Normal
    this->setName("Normal");

    if (_sprite) {
        _sprite->stopActionByTag(TAG_ANIMATION);
    }

    switch (newState)
    {
    case State::Idle:
        playAnimation("boss/idle/", 1, 5, true, 0.1f);
        _stateTimer = cocos2d::random(0.6f, 1.0f);
        break;

    case State::Falling_Enter:
        playAnimation("boss/fall/", 1, 7, true, 0.1f);
        break;

    case State::Pre_Jump:
        switchState(State::Jumping);
        break;

    case State::Jumping:
    {
        playAnimation("boss/jump/", 1, 10, false, 0.1f);
        _velocity.y = JUMP_FORCE_NORMAL;

        float airTime = 2.0f * JUMP_FORCE_NORMAL / std::abs(GRAVITY);
        float distanceToPlayer = _lastPlayerX - this->getPositionX();
        float requiredVx = distanceToPlayer / airTime;

        _velocity.x = std::max(-MAX_JUMP_SPEED_X, std::min(MAX_JUMP_SPEED_X, requiredVx));
        break;
    }

    case State::Pre_Attack:
        switchState(State::Jump_Attack);
        break;

    case State::Jump_Attack:
    {
        _isAttackLanded = false;
        _isHammerActive = false;

        if (_sprite) _sprite->stopActionByTag(TAG_ANIMATION);

        Vector<SpriteFrame*> riseFrames; // 1-3
        for (int i = 1; i <= 3; i++) {
            auto f = Sprite::create("boss/jumpAttack/" + std::to_string(i) + ".png");
            if (f) riseFrames.pushBack(f->getSpriteFrame());
        }
        auto riseAnim = Animate::create(Animation::createWithSpriteFrames(riseFrames, 0.1f));

        Vector<SpriteFrame*> hangFrames; // 4-5
        for (int i = 4; i <= 5; i++) {
            auto f = Sprite::create("boss/jumpAttack/" + std::to_string(i) + ".png");
            if (f) hangFrames.pushBack(f->getSpriteFrame());
        }
        auto hangAnim = Animate::create(Animation::createWithSpriteFrames(hangFrames, 0.6f));

        auto seq = Sequence::create(riseAnim, hangAnim, nullptr);
        seq->setTag(TAG_ANIMATION);
        _sprite->runAction(seq);

        _velocity.y = JUMP_FORCE_ATTACK;

        float airTime = 2.0f * JUMP_FORCE_ATTACK / std::abs(GRAVITY);
        float distanceToPlayer = _lastPlayerX - this->getPositionX();
        float requiredVx = distanceToPlayer / airTime;

        _velocity.x = std::max(-MAX_ATTACK_SPEED_X, std::min(MAX_ATTACK_SPEED_X, requiredVx));
        break;
    }

    case State::Rampage_Jump:
    {
        playAnimation("boss/jump/", 1, 10, false, 0.1f);
        _velocity.y = RAMPAGE_JUMP_FORCE;

        float targetX = 1650.0f;
        float airTime = 2.0f * RAMPAGE_JUMP_FORCE / std::abs(GRAVITY);
        float distanceToTarget = targetX - this->getPositionX();
        _velocity.x = distanceToTarget / airTime;
        break;
    }

    case State::Rampage_Attack:
    {
        rampageAttack();
        break;
    }

    case State::Shockwave_Attack:
    {
        _velocity = Vec2::ZERO;
        _isAttackLanded = false;
        _isHammerActive = false;

        Vector<SpriteFrame*> preFrames;
        for (int i = 1; i <= 6; ++i)
        {
            auto f = Sprite::create("boss/shockwaveAttack/" + std::to_string(i) + ".png");
            if (f) preFrames.pushBack(f->getSpriteFrame());
        }

        Vector<SpriteFrame*> postFrames;
        for (int i = 7; i <= 14; ++i)
        {
            auto f = Sprite::create("boss/shockwaveAttack/" + std::to_string(i) + ".png");
            if (f) postFrames.pushBack(f->getSpriteFrame());
        }

        if (!preFrames.empty() && !postFrames.empty())
        {
            auto animPre = Animate::create(Animation::createWithSpriteFrames(preFrames, SHOCKWAVE_FRAME_DELAY));
            auto animPost = Animate::create(Animation::createWithSpriteFrames(postFrames, SHOCKWAVE_FRAME_DELAY));

            auto seq = Sequence::create(animPre, animPre->clone(), animPost, CallFunc::create([this]() {
                switchState(State::Idle);
                }), nullptr);

            seq->setTag(TAG_ANIMATION);
            _sprite->runAction(seq);
        }

        const float startDelay = (6 * SHOCKWAVE_FRAME_DELAY * 2) + (2 * SHOCKWAVE_FRAME_DELAY);

        this->runAction(Sequence::create(
            DelayTime::create(startDelay),
            CallFunc::create([this]() {
                _isHammerActive = true;
                if (_shockwaveCallback)
                {
                    float dir = (_facing >= 0) ? 1.0f : -1.0f;
                    Vec2 spawnPos = this->getPosition() + Vec2(dir * SHOCKWAVE_SPAWN_OFFSET_X * BOSS_SCALE, SHOCKWAVE_SPAWN_OFFSET_Y * BOSS_SCALE);
                    _shockwaveCallback(spawnPos, dir);
                }
                }),
            DelayTime::create(SHOCKWAVE_FRAME_DELAY),
            CallFunc::create([this]() { _isHammerActive = false; }),
            nullptr));
        break;
    }

    case State::Stunned:
    {
        // 标记为 "Stunned" 状态（外部碰撞逻辑可用 getName() 判断）
        this->setName("Stunned");

        _velocity.x = 0;
        _stunHitCount = 0; // 每次进入瘫痪窗口：重置“本次窗口累计伤害”
        _stunTimer = STUN_DURATION;
        _isStunAnimPlaying = true;

        playAnimation("boss/recovery/", 1, 14, false, 0.12f, [this]() {
            _isStunAnimPlaying = false;
            auto frame = Sprite::create("boss/recovery/14.png")->getSpriteFrame();
            _sprite->setSpriteFrame(frame);
            });
        break;
    }

    case State::Recovering:
        recoverFromStun();
        break;
    }
}

void Boss::recoverFromStun()
{
    CCLOG("Boss Recovering from Stun (early)...");

    _hitCount = 0;
    _actionStep = 0;
    startRampage();
}

void Boss::startRampage()
{
    CCLOG("Boss starts rampage!");
    this->setName("Rampage");
    switchState(State::Rampage_Jump);
}

void Boss::rampageAttack()
{
    _rampageCounter = 0;
    _isRampaging = true;

    playAnimation("boss/rampageAttack/", 1, 6, false, 0.1f, [this]() {
        rampageAttackLoop(0);
        });
}

void Boss::rampageAttackLoop(int count)
{
    if (count >= RAMPAGE_ATTACK_COUNT) {
        _isRampaging = false;
        _actionStep = 0;
        switchState(State::Idle);
        return;
    }

    float newFacing = (count % 2 == 0) ? -1.0f : 1.0f;
    applyFacing(newFacing);

    playAnimation("boss/rampageAttack/", 7, 6, false, 0.1f, [this, count]() {
        rampageAttackLoop(count + 1);
        });

    _isHammerActive = false;
    const float frameDelay = 0.1f;
    const float startDelay = frameDelay * (9 - 7);
    const float activeDuration = frameDelay;
    scheduleHammerWindow(startDelay, activeDuration);
}

void Boss::onLand()
{
    if (_state == State::Falling_Enter || _state == State::Jumping)
    {
        _velocity = Vec2(0, 0);
        switchState(State::Idle);
    }
    else if (_state == State::Rampage_Jump)
    {
        _velocity = Vec2(0, 0);
        switchState(State::Rampage_Attack);
    }
    else if (_state == State::Jump_Attack)
    {
        if (_isAttackLanded) return;
        _isAttackLanded = true;

        _velocity = Vec2(0, 0);

        if (_sprite) _sprite->stopActionByTag(TAG_ANIMATION);

        _isHammerActive = false;

        Vector<SpriteFrame*> frame6;
        auto f6 = Sprite::create("boss/jumpAttack/6.png");
        if (f6) frame6.pushBack(f6->getSpriteFrame());
        auto anim6 = Animate::create(Animation::createWithSpriteFrames(frame6, 0.05f));

        Vector<SpriteFrame*> frame7;
        auto f7 = Sprite::create("boss/jumpAttack/7.png");
        if (f7) frame7.pushBack(f7->getSpriteFrame());
        auto anim7 = Animate::create(Animation::createWithSpriteFrames(frame7, 0.1f));

        Vector<SpriteFrame*> recoverFrames;
        for (int i = 8; i <= 11; i++) {
            std::string path = "boss/jumpAttack/" + std::to_string(i) + ".png";
            auto f = Sprite::create(path);
            if (f) recoverFrames.pushBack(f->getSpriteFrame());
        }
        auto recoverAnim = Animate::create(Animation::createWithSpriteFrames(recoverFrames, 0.1f));

        auto seq = Sequence::create(
            CallFunc::create([this]() { _isHammerActive = false; }),
            anim6,

            CallFunc::create([this]() {
                _isHammerActive = true;
                }),
            anim7,

            CallFunc::create([this]() {
                _isHammerActive = false;
                this->setName("Recovery");
                }),
            recoverAnim,

            CallFunc::create([this]() { switchState(State::Idle); }),
            nullptr
        );
        seq->setTag(TAG_ANIMATION);
        _sprite->runAction(seq);
    }
}

void Boss::setFacing(float playerX)
{
    float x = this->getPositionX();
    if (std::abs(playerX - x) > 10.0f) {
        float newFacing = (playerX > x) ? 1.0f : -1.0f;
        applyFacing(newFacing);
    }
}

void Boss::applyFacing(float newFacing)
{
    if (!_sprite) return;

    float normalized = (newFacing >= 0) ? 1.0f : -1.0f;
    if (normalized == _facing) return;

    float forwardOffset = getForwardOffset();
    this->setPositionX(this->getPositionX() - forwardOffset * (normalized - _facing));

    _facing = normalized;
    _sprite->setScaleX(BOSS_SCALE * _facing);
}

void Boss::scheduleHammerWindow(float startDelay, float duration)
{
    this->stopActionByTag(TAG_HAMMER_WINDOW);
    auto seq = Sequence::create(
        DelayTime::create(startDelay),
        CallFunc::create([this]() { _isHammerActive = true; }),
        DelayTime::create(duration),
        CallFunc::create([this]() { _isHammerActive = false; }),
        nullptr);
    seq->setTag(TAG_HAMMER_WINDOW);
    this->runAction(seq);
}

void Boss::playAnimation(std::string folder, int startFrame, int frameCount, bool loop, float delay, std::function<void()> onComplete)
{
    Vector<SpriteFrame*> frames;
    int endFrame = startFrame + frameCount - 1;

    for (int i = startFrame; i <= endFrame; i++) {
        std::string path = folder + std::to_string(i) + ".png";
        auto spriteFrame = Sprite::create(path);
        if (spriteFrame) {
            frames.pushBack(spriteFrame->getSpriteFrame());
        }
    }

    if (frames.empty()) return;

    Animation* animation = Animation::createWithSpriteFrames(frames, delay);
    Animate* animate = Animate::create(animation);

    Action* action = nullptr;

    if (loop) {
        action = RepeatForever::create(animate);
    }
    else {
        if (onComplete) {
            action = Sequence::create(animate, CallFunc::create(onComplete), nullptr);
        }
        else {
            action = animate;
        }
    }

    if (action) {
        action->setTag(TAG_ANIMATION);
        _sprite->runAction(action);
    }
}

void Boss::takeDamage(int damage)
{
    if (_hurtTimer > 0) return;
    if (_state == State::Stunned && _isStunAnimPlaying) return;

    _hurtTimer = HURT_COOLDOWN;
    flashEffect();

    // ====== 新增：受击打击感特效（缩小并右下偏移，偏移减小） ======
    if (_sprite) {
        float fxSize = std::max(_sprite->getContentSize().width, _sprite->getContentSize().height) * BOSS_SCALE * 0.55f; // 稍大一点
        // 调整到Boss中部偏下且稍微偏右
        float yOffset = _sprite->getContentSize().height * BOSS_SCALE * 0.28f; // 0.28为中部偏下
        float xOffset = _sprite->getContentSize().width * BOSS_SCALE * 0.05f;  // 0.10为稍微偏右
        Vec2 offset = Vec2(xOffset, yOffset);
        HitEffect::play(this->getParent(), this->getPosition() + offset, fxSize);
    }
    // ===============================

    if (_state == State::Stunned)
    {
        // 假骑士( False Knight )式：只有在瘫痪(Stunned)期间受击才会“掉血”
        // damage: 平A=1, 法术=2
        _stunHitCount += damage; // 本次瘫痪窗口内累计伤害点数
        _stunHP -= damage;       // 总血量只在 Stunned 内扣

        // 血条清空：立刻死亡
        if (_stunHP <= 0)
        {
            die();
            return;
        }

        // 血条未清空：单次瘫痪内累计伤害达到阈值，立刻起身
        if (_stunHitCount >= MAX_STUN_HITS)
        {
            recoverFromStun();
        }
    }
    else if (_state != State::Recovering && _state != State::Falling_Enter)
    {
        _hitCount += damage;
        if (_hitCount >= MAX_HITS_BEFORE_STUN) {
            switchState(State::Stunned);
        }
    }
}

void Boss::flashEffect()
{
    if (!_sprite) return;
    _sprite->stopActionByTag(TAG_FLASH);

    auto toRed = TintTo::create(0.05f, 255, 100, 100);
    auto toWhite = TintTo::create(0.05f, 255, 255, 255);
    auto seq = Sequence::create(toRed, toWhite, nullptr);

    seq->setTag(TAG_FLASH);
    _sprite->runAction(seq);
}

Rect Boss::getBodyHitbox() const
{
    // 如果已死亡，不再有碰撞体积
    if (_isDead) return Rect::ZERO;

    if (!_sprite) return Rect::ZERO;
    Size size = _sprite->getContentSize();
    Vec2 pos = this->getPosition();

    float dir = _facing;

    float widthRatio = 0.440f;
    if (_state == State::Jump_Attack && this->getName() != "Recovery") {
        widthRatio = 0.44f;
    }

    float forwardOffset = getForwardOffset();

    float w = size.width * BOSS_SCALE * widthRatio;
    float h = size.height * BOSS_SCALE * 0.7f;

    return Rect(pos.x - w / 2 + (forwardOffset * dir), pos.y, w, h);
}

Rect Boss::getHammerHitbox() const
{
    // 如果已死亡，不再有碰撞体积
    if (_isDead) return Rect::ZERO;

    if ((_state == State::Jump_Attack || _state == State::Rampage_Attack || _state == State::Shockwave_Attack) && _onGround && _isHammerActive)
    {
        Vec2 pos = this->getPosition();
        float dir = _facing;

        float hammerW = 180.0f * BOSS_SCALE;
        float hammerH = 150.0f * BOSS_SCALE;

        float offsetX = dir * 140.0f * BOSS_SCALE;

        return Rect(pos.x + offsetX - hammerW / 2, pos.y, hammerW, hammerH);
    }
    return Rect::ZERO;
}

float Boss::getForwardOffset() const
{
    if (_state == State::Jump_Attack && this->getName() != "Recovery") {
        return 120.0f * BOSS_SCALE;
    }
    return 80.0f;
}

void Boss::die()
{
    if (_isDead) return;

    _isDead = true;
    _isHammerActive = false;
    _velocity = Vec2::ZERO;
    this->setName("Dead");

    // 停止所有动作（避免继续攻击/位移）
    this->stopAllActions();
    if (_sprite) _sprite->stopAllActions();

    // 安全的“淡出+隐藏”
    if (_sprite)
    {
        _sprite->runAction(Sequence::create(
            FadeOut::create(0.25f),
            CallFunc::create([this]() {
                this->setVisible(false);
                }),
            nullptr));
    }
    else
    {
        this->setVisible(false);
    }
}