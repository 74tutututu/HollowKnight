#include "HUDLayer.h"
#include "config.h" 

USING_NS_CC;

HUDLayer* HUDLayer::createLayer()
{
    HUDLayer* layer = HUDLayer::create();
    return layer;
}

bool HUDLayer::init()
{
    if (!Layer::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 1. 创建容器
    _healthBarContainer = Node::create();
    _healthBarContainer->setScale(2.5f);
    // 容器整体位置
    _healthBarContainer->setPosition(Vec2(250.0f, visibleSize.height - 200.0f));
    this->addChild(_healthBarContainer);

    // ============================================================
    // 定义统一的坐标和缩放
    // ============================================================
    Vec2 soulPos = Vec2(-25.0f, -45.0f);
    float soulScale = 2.9f;

    // ============================================================
    // 2. 创建外框 (Frame) - 初始显示第1帧
    // ============================================================
    std::string startFrame = StringUtils::format(Config::Soul::PATH_FRAME_ANIM.c_str(), 1);
    _soulFrame = Sprite::create(startFrame);

    if (_soulFrame)
    {
        _soulFrame->setPosition(30,-40);   // 必须对齐
        _healthBarContainer->addChild(_soulFrame, 1); // Z=1 (底层)
    }

    // ============================================================
    // 3. 创建液体 (Orb)
    // ============================================================
    _soulOrb = Sprite::create(Config::Soul::PATH_EMPTY);

    if (_soulOrb)
    {
        _soulOrb->setPosition(soulPos);     
        _soulOrb->setScale(soulScale);    
        _healthBarContainer->addChild(_soulOrb, 2); // Z=2 (顶层)
    }

    // 此时不创建血条，而是启动开场动画
    this->playOpeningSequence();

    return true;
}

// ============================================================
// 开场动画：先长出框，再长出血条
// ============================================================
void HUDLayer::playOpeningSequence()
{
    if (!_soulFrame) return;

    // 1. 准备外框生长动画 (Frame 1 -> 6)
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= Config::Soul::FRAMES_FRAME_COUNT; i++)
    {
        std::string name = StringUtils::format(Config::Soul::PATH_FRAME_ANIM.c_str(), i);
        auto sprite = Sprite::create(name);
        if (sprite) frames.pushBack(sprite->getSpriteFrame());
    }

    if (frames.empty()) return;

    // 播放速度
    auto animation = Animation::createWithSpriteFrames(frames, 0.1f);
    animation->setRestoreOriginalFrame(false); // 不还原，停在最后一帧
    auto animate = Animate::create(animation);

    // 2. 动画结束回调
    auto finishCallback = CallFunc::create([this]() {
        // 强制换成静态图 (定格在第6张)
        this->_soulFrame->setTexture(Config::Soul::PATH_FRAME_STATIC);

        // 开始生成血条 (从第0个开始，假设总共5个)
        // 这里的 5 最好是从 Player 传进来的 MaxHP
        this->spawnNextHealth(0, 5);
        });

    // 3. 运行动作
    _soulFrame->runAction(Sequence::create(animate, finishCallback, nullptr));
}

// ============================================================
// 递归生成血条：一个接一个出现
// ============================================================
void HUDLayer::spawnNextHealth(int index, int maxHp)
{
    // 终止条件
    if (index >= maxHp) {
        // 全部生成完毕后，强制刷新一次 UI，确保状态正确
        // 这里需要获取主角当前的真实血量，如果没有传进来，可以先全部设为满
        this->updateHealth(maxHp, maxHp);
        return;
    }

    // 1. 创建血条 Sprite (初始用 appear_1)
    std::string startImg = StringUtils::format(Config::Health::PATH_APPEAR.c_str(), 1);
    auto heart = Sprite::create(startImg);
    if (!heart) return;

    // 2. 设置位置 (和 updateHealth 逻辑保持一致)
    // 注意：这里的 Y 坐标可能需要根据你的实际情况微调
    heart->setPosition(Vec2(index * 50.0f + 80, -10));
    heart->setTag(1); // 标记为满血
    _healthBarContainer->addChild(heart, 10);
    _heartSprites.push_back(heart);

    // 3. 准备 Appear 动画 (1 -> 5)
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= Config::Health::FRAMES_APPEAR; i++)
    {
        std::string name = StringUtils::format(Config::Health::PATH_APPEAR.c_str(), i);
        auto sprite = Sprite::create(name);
        if (sprite) frames.pushBack(sprite->getSpriteFrame());
    }

    // 保护
    if (frames.empty()) return;

    auto animation = Animation::createWithSpriteFrames(frames, 0.06f);
    animation->setRestoreOriginalFrame(false);
    auto animate = Animate::create(animation);

    // 4. 回调：当前动画播完，生成下一个
    auto finishCallback = CallFunc::create([this, index, maxHp, heart]() {
        // 播完后换成静态 Full 图
        heart->setTexture(Config::Health::PATH_FULL);

        // 【递归】生成下一个
        this->spawnNextHealth(index + 1, maxHp);
        });

    // 5. 运行
    heart->runAction(Sequence::create(animate, finishCallback, nullptr));
}

void HUDLayer::updateHealth(int currentHp, int maxHp)
{
    if (_heartSprites.size() != maxHp) return;

    // --------------------------------------------------------
    // B. 更新状态
    // --------------------------------------------------------
    for (int i = 0; i < _heartSprites.size(); i++)
    {
        auto heart = _heartSprites[i];
        int currentTag = heart->getTag(); // 获取当前记录的状态

        // 情况 1: 这一格应该有血
        if (i < currentHp)
        {
            // 如果之前不是满血，或者是第一次刷新
            if (currentTag != 1)
            {
                heart->setTexture(Config::Health::PATH_FULL);
                heart->setOpacity(255);
                heart->setTag(1); // 标记为满血
            }
        }
        // 情况 2: 这一格应该没血
        else
        {
            // 检查：它之前是否满血
            // 如果 Tag 是 1，说明它刚刚还在，现在没了 ,触发破碎
            if (currentTag == 1)
            {
                // 1. 播放破碎动画 (盖在上面)
                playBreakAnimation(heart);

                // 2. 自身立刻切换为空血槽 (作为背景垫在下面)
                heart->setTexture(Config::Health::PATH_EMPTY);

                // 3. 标记为空血
                heart->setTag(0);
            }
            
        }
    }
}

void HUDLayer::playBreakAnimation(Sprite* heartSprite)
{
    // 1. 加载动画帧
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= 6; i++)
    {
        std::string name = StringUtils::format(Config::Health::PATH_BREAK.c_str() , i);
        auto sprite = Sprite::create(name);
        if (sprite) {
            frames.pushBack(sprite->getSpriteFrame());
        }
    }

    if (frames.empty()) return;

    // 2. 创建动画 (速度快一点，0.04秒一帧)
    auto animation = Animation::createWithSpriteFrames(frames, 0.06f);
    auto animate = Animate::create(animation);

    // 3. 创建临时 Sprite
    // 因为所有图片尺寸一样，直接对齐坐标即可，无需偏移
    auto effectSprite = Sprite::create();
    effectSprite->setPosition(heartSprite->getPosition());

    // Z序设为 20，保证盖在空血槽上面
    _healthBarContainer->addChild(effectSprite, 20);

    // 4. 播放并销毁
    auto seq = Sequence::create(
        animate,
        RemoveSelf::create(),
        nullptr
    );

    effectSprite->runAction(seq);
}

// 【核心函数】创建灵魂瓶 "动画 -> 等待 -> 循环" 的动作
Action* HUDLayer::createSoulAction(int soulValue)
{
    std::string pathFormat;
    int frameCount = 0;

    // 0 不需要动画，所以从 1 开始
    switch (soulValue)
    {
    case 1: // 1/4 (Quarter)
        pathFormat = Config::Soul::PATH_QUARTER;
        frameCount = Config::Soul::FRAMES_QUARTER;
        break;
    case 2: // 1/2 (Half)
        pathFormat = Config::Soul::PATH_HALF;
        frameCount = Config::Soul::FRAMES_HALF;
        break;
    case 3: // 3/4 (3Quarter)
        pathFormat = Config::Soul::PATH_3QUARTER;
        frameCount = Config::Soul::FRAMES_3QUARTER;
        break;
    case 4: // Full
        pathFormat = Config::Soul::PATH_FULL;
        frameCount = Config::Soul::FRAMES_FULL;
        break;
    default:
        return nullptr;
    }

    // 加载动画帧
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= frameCount; i++)
    {
        std::string name = StringUtils::format(pathFormat.c_str(), i);
        auto sprite = Sprite::create(name);
        if (sprite) {
            frames.pushBack(sprite->getSpriteFrame());
        }
    }

    if (frames.empty()) return nullptr;

    auto animation = Animation::createWithSpriteFrames(frames, Config::Soul::FRAME_SPEED);
    auto animate = Animate::create(animation);

    // 逻辑：满魂(4)需要呼吸等待，其他状态(1,2,3)无限循环
    if (soulValue == 4)
    {
        auto delay = DelayTime::create(Config::Soul::LOOP_DELAY);
        auto seq = Sequence::create(animate, delay, nullptr);
        return RepeatForever::create(seq);
    }
    else
    {
        return RepeatForever::create(animate);
    }
}

// --------------------------------------------------------
// 核心：更新灵魂显示
// --------------------------------------------------------
void HUDLayer::updateSoul(int currentSoul)
{
    // 1. 防抽搐：如果数值没变，直接忽略
    if (currentSoul == _lastSoul) return;

    // 2. 边界钳制
    int safeSoul = currentSoul;
    if (safeSoul < 0) safeSoul = 0;
    if (safeSoul > 4) safeSoul = 4;

    // 更新记录
    _lastSoul = safeSoul;

    //  3. 停止之前的动画
    _soulOrb->stopAllActions();

    //  4. 分情况处理
    if (safeSoul == 0)
    {
        // 空瓶：显示静止图
        _soulOrb->setTexture(Config::Soul::PATH_EMPTY);

        // 确保它不透明 (防止之前某些动画改了透明度)
        _soulOrb->setOpacity(255);
    }
    else
    {
        // 有魂：播放动画
        Action* action = createSoulAction(currentSoul);
        if (action)
        {
            _soulOrb->runAction(action);
        }

    }
}