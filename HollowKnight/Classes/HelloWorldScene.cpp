#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "Enemy.h"
#include "Zombie.h"
#include "Spike.h"
#include "Buzzer.h"
#include "HUDLayer.h"
#include "Jar.h"
#include "Fireball.h" 
#include "PlayerAnimator.h"
#include "config.h"
#include "KeyBindingScene.h"  
#include "Boss.h"  
#include "FKFireball.h" 
#include "FKShockwave.h"
#include "DreamDialogue.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

void HelloWorld::parseMapCollisions(TMXTiledMap* map)
{
    _groundRects.clear(); // 先清空

    // 1. 获取对象层
    auto objectGroup = map->getObjectGroup("collision");

    if (objectGroup == nullptr) {
        CCLOG("Error: 'collision' object group not found in TMX map!");
        return;
    }

    // 2. 获取层里所有的对象
    auto& objects = objectGroup->getObjects();

    for (auto& obj : objects)
    {
        // obj 是一个 ValueMap (键值对字典)
        cocos2d::ValueMap& dict = obj.asValueMap();

        // 3. 读取坐标和宽高
        float x = dict["x"].asFloat();
        float y = dict["y"].asFloat();
        float w = dict["width"].asFloat();
        float h = dict["height"].asFloat();

        // 4. 创建 Rect 并存起来
        Rect rect = Rect(x, y, w, h);
        _groundRects.push_back(rect);

        // CCLOG("Parsed Ground Rect: x=%f, y=%f, w=%f, h=%f", x, y, w, h);
    }
}

bool HelloWorld::init()
{
    if (!Scene::init())
    {
        return false;
    }

    _isGamePaused = false;
    _pauseLayer = nullptr;

    PlayerAnimator::preloadSounds();

    // 播放背景音乐 (使用 Config)
    auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
    audio->playBackgroundMusic(Config::Audio::BGM_DIRTMOUTH, true);
    audio->setBackgroundMusicVolume(0.5f);
    audio->setEffectsVolume(0.8f);

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ============================================================
    // 1. 创建游戏容器层
    // ============================================================
    _gameLayer = Layer::create();

    // 把缩放应用在 GameLayer 上，而不是整个 Scene
    // 这样 UI 就不会被放大，保持高清！
    float zoomScale = 1.5f;
    _gameLayer->setScale(zoomScale);

    // 游戏层 Z序低 (1)，放在下面
    this->addChild(_gameLayer, 1);

    //////////////////////////////////////////////////////////////////////
    // 2. 背景
    //////////////////////////////////////////////////////////////////////
    auto bgLayer = LayerColor::create(Color4B(40, 40, 40, 255)); // 稍微调暗一点，更有氛围
    _gameLayer->addChild(bgLayer, -100);

    //////////////////////////////////////////////////////////////////////
    // 3. 使用loadMap方法加载level1
    //////////////////////////////////////////////////////////////////////
    loadMap("maps/level1.tmx");

    //////////////////////////////////////////////////////////////////////
    // 4. 在 init 中创建 Level 1 特有的敌人
    //////////////////////////////////////////////////////////////////////

    // --- 创建 Enemy ---
    auto enemy = Enemy::create("enemies/enemy_walk_1.png");
    if (enemy)
    {
        enemy->setPosition(Vec2(600, 430));
        enemy->setPatrolRange(500, 800);
        enemy->setTag(999);
        _gameLayer->addChild(enemy, 5);

        // 死亡回调 (回魂 + 音效)
        enemy->setOnDeathCallback([=]() {
            if (_player) {
                _player->gainSoulOnKill();
                CCLOG("Soul gained from Enemy!");
            }
            // SimpleAudioEngine::getInstance()->playEffect("audio/enemy_death.wav");
            });

        CCLOG("Enemy spawned!");
    }

    // --- 创建 Zombie 敌人 ---
    auto zombie = Zombie::create("zombie/walk/walk_1.png");
    if (zombie)
    {
        zombie->setPosition(Vec2(1200, 430));
        zombie->setPatrolRange(1000, 1400);
        zombie->setTag(998);
        _gameLayer->addChild(zombie, 5);

        // 死亡回调
        zombie->setOnDeathCallback([=]() {
            if (_player) {
                // 兼容不同写法，这里假设 Player 有 gainSoulOnKill
                _player->gainSoulOnKill();
                CCLOG("Soul gained from Zombie!");
            }
            // SimpleAudioEngine::getInstance()->playEffect("audio/enemy_death.wav");
            });

        CCLOG("Zombie spawned at position (1200, 430)!");
    }

    // --- 创建 Spike 陷阱 ---
    auto textureCache = Director::getInstance()->getTextureCache();
    auto spikeTexture = textureCache->addImage("traps/spike.png");
    Spike* spike = nullptr;

    if (spikeTexture) {
        spike = Spike::create("traps/spike.png");
    }
    else {
        spike = Spike::create("enemies/enemy_walk_1.png"); // Fallback
    }

    if (spike)
    {
        Vec2 spikePos = Vec2(3590.0f, 1000.0f);
        spike->setInitialPosition(spikePos);
        spike->setTag(997);
        spike->setAnchorPoint(Vec2(0.5f, 0.5f));
        spike->setVisible(true);
        spike->setOpacity(255);
        spike->setScale(1.0f);
        spike->setBlendFunc(BlendFunc::ALPHA_PREMULTIPLIED);
        _gameLayer->addChild(spike, 5);
    }

    // --- 创建 Buzzer 飞行敌人 ---
    auto buzzer1 = Buzzer::create("buzzer/idle/idle_1.png");
    if (buzzer1)
    {
        Vec2 buzzer1Pos = Vec2(4000.0f, 900.0f);
        buzzer1->setInitialPosition(buzzer1Pos);
        buzzer1->setTag(996);
        _gameLayer->addChild(buzzer1, 5);

        buzzer1->setOnDeathCallback([=]() {
            if (_player) {
                _player->gainSoulOnKill(); // 调用主角加魂
                CCLOG("Soul gained from Buzzer 1!");
            }
            });
    }

    auto buzzer2 = Buzzer::create("buzzer/idle/idle_1.png");
    if (buzzer2)
    {
        Vec2 buzzer2Pos = Vec2(6000.0f, 700.0f);
        buzzer2->setInitialPosition(buzzer2Pos);
        buzzer2->setTag(995);
        _gameLayer->addChild(buzzer2, 5);

        buzzer2->setOnDeathCallback([=]() {
            if (_player) {
                _player->gainSoulOnKill(); // 调用主角加魂
                CCLOG("Soul gained from Buzzer 2!");
            }
            });
    }

    //////////////////////////////////////////////////////////////////////
    // 5. 创建主角 (Player)
    //////////////////////////////////////////////////////////////////////
    _player = Player::create("Knight/idle/idle_1.png");

    if (_player)
    {
        _player->setPosition(Vec2(450, 1300));  
        _gameLayer->addChild(_player, 10);
        CCLOG("Player created successfully!");
    }
    else
    {
        CCLOG("Error: Failed to create Player!");
    }

    //////////////////////////////////////////////////////////////////////
    // 6. 创建 UI 层
    //////////////////////////////////////////////////////////////////////
    auto hudLayer = HUDLayer::createLayer();
    hudLayer->setTag(900);
    this->addChild(hudLayer, 100);

    if (_player) {
        // 血量监听
        _player->setOnHealthChanged([=](int hp, int maxHp) {
            hudLayer->updateHealth(hp, maxHp);
            });

        // 魂量监听 
        _player->setOnSoulChanged([=](int soul) {
            hudLayer->updateSoul(soul);
            });

        // 初始化 UI
        hudLayer->updateHealth(_player->getHealth(), _player->getMaxHealth());
    }

    //////////////////////////////////////////////////////////////////////
    // 7. 键盘监听器
    //////////////////////////////////////////////////////////////////////
    auto listener = EventListenerKeyboard::create();
    // 【重要】不要在这里捕获 kbm，而是在每次回调中动态获取

    // --- 按下按键 ---
    listener->onKeyPressed = [=](EventKeyboard::KeyCode code, Event* event) {
        if (_player == nullptr) return;

        // 【修复】每次都重新获取 KeyBindingManager 实例，确保使用最新的键位配置
        auto kbm = KeyBindingManager::getInstance();

        // ============================================================
        // 使用配置的 PAUSE 键来暂停
        // ============================================================
        if (code == kbm->getKeyForAction(KeyBindingManager::Action::PAUSE))
        {
            auto pauseScene = KeyBindingScene::create();
            if (pauseScene)
            {
                pauseScene->setPauseMode(true);
                Director::getInstance()->pushScene(pauseScene);
                return;
            }
        }

        // 使用键位管理器检查按键
        if (code == kbm->getKeyForAction(KeyBindingManager::Action::MOVE_RIGHT))
        {
            _isRightPressed = true;
            updatePlayerMovement();
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::MOVE_LEFT))
        {
            _isLeftPressed = true;
            updatePlayerMovement();
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::MOVE_UP))
        {
            _isUpPressed = true;
            updatePlayerMovement();
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::MOVE_DOWN))
        {
            _isDownPressed = true;
            updatePlayerMovement();
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::JUMP))
        {
            _player->setJumpPressed(true);
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::ATTACK))
        {
            _player->setAttackPressed(true);
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::FOCUS))
        {
            _player->setFocusInput(true);
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::CAST_SPELL))
        {
            _player->setCastInput(true);
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::DREAM_NAIL))
        {
            _player->setDreamNailInput(true);
        }
        };

    // --- 松开按键 ---
    listener->onKeyReleased = [=](EventKeyboard::KeyCode code, Event* event) {
        if (_player == nullptr) return;

        // 【修复】每次都重新获取 KeyBindingManager 实例
        auto kbm = KeyBindingManager::getInstance();

        // 使用键位管理器检查按键
        if (code == kbm->getKeyForAction(KeyBindingManager::Action::FOCUS))
        {
            _player->setFocusInput(false);
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::MOVE_LEFT))
        {
            _isLeftPressed = false;
            updatePlayerMovement();
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::MOVE_RIGHT))
        {
            _isRightPressed = false;
            updatePlayerMovement();
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::MOVE_UP))
        {
            _isUpPressed = false;
            updatePlayerMovement();
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::MOVE_DOWN))
        {
            _isDownPressed = false;
            updatePlayerMovement();
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::JUMP))
        {
            _player->setJumpPressed(false);
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::ATTACK))
        {
            _player->setAttackPressed(false);
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::CAST_SPELL))
        {
            _player->setCastInput(false);
        }
        else if (code == kbm->getKeyForAction(KeyBindingManager::Action::DREAM_NAIL))
        {
            _player->setDreamNailInput(false);
        }
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    //////////////////////////////////////////////////////////////////////
    // 8. 退出按钮
    //////////////////////////////////////////////////////////////////////
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem)
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);

    //////////////////////////////////////////////////////////////////////
    // 9. 调试 UI
    //////////////////////////////////////////////////////////////////////
    //_coordLabel = Label::createWithSystemFont("Player: (0, 0)", "Arial", 24);
    //_coordLabel->setColor(Color3B::YELLOW);
    //_coordLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
    //_coordLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    //this->addChild(_coordLabel, 200);

    //_spikeDebugLabel = Label::createWithSystemFont("Spike: Loading...", "Arial", 20);
    //_spikeDebugLabel->setColor(Color3B::RED);
    //_spikeDebugLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    //_spikeDebugLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
    //this->addChild(_spikeDebugLabel, 200);

    //_debugDrawNode = DrawNode::create();
    //this->addChild(_debugDrawNode, 150);

    _coordLabel = nullptr;
    _spikeDebugLabel = nullptr;
    _debugDrawNode = nullptr;

    this->scheduleUpdate();

    return true;
}

void HelloWorld::updatePlayerMovement()
{
    if (!_player) return;

    // 左键(-1) + 右键(+1)
    int dirX = 0, dirY = 0;
    if (_isLeftPressed)  dirX -= 1;
    if (_isRightPressed) dirX += 1;

    if (_isUpPressed) dirY += 1;
    if (_isDownPressed) dirY -= 1;

    _player->setInputDirectionX(dirX);
    _player->setInputDirectionY(dirY);
}

// 每帧更新：实现相机跟随、碰撞检测、关卡切换
void HelloWorld::update(float dt)
{
    if (!_player || !_gameLayer) return;

    auto map = _gameLayer->getChildByTag(123);
    if (!map) return;

    // ========================================
    // 0. 检测玩家位置，触发场景切换 (Level 1 -> 2)
    // ========================================
 // Level 1 -> Level 2
    if (_currentLevel == 1 && !_isTransitioning)
    {
        Vec2 playerPos = _player->getPosition();
        if (playerPos.x >= 6500.0f)
        {
            CCLOG("Player reached level1 end! Triggering level switch...");
            switchToLevel2();
            return;
        }
    }
    // Level 2 -> Level 1
    if (_currentLevel == 2 && !_isTransitioning)
    {
        Vec2 playerPos = _player->getPosition();
        if (playerPos.x <= 100.0f)
        {
            CCLOG("Player reached level2 left! Triggering level 1 switch...");
            switchToLevel1();
            return;
        }
        if (playerPos.x >= 6325.0f)
        {
            CCLOG("Player reached level2 end! Triggering level 3 switch...");
            switchToLevel3();
            return;
        }
    }
    // Level 3 -> Level 2
    if (_currentLevel == 3 && !_isTransitioning)
    {
        Vec2 playerPos = _player->getPosition();
        if (playerPos.x <= 100.0f)
        {
            CCLOG("Player reached level3 left! Triggering level 2 switch...");
            switchToLevel2FromRight();
            return;
        }
    }


    // ========================================
    // 1. 更新玩家位置 (包含 Jar 平台逻辑)
    // ========================================
   if (_jars.empty())
   {
       _player->update(dt, _groundRects); // ⚡️ 零拷贝，极速！
   }
   else
   {
       // 只有 Level 2 有罐子时，才不得不复制一份
       std::vector<Rect> dynamicGroundRects = _groundRects;

       // 倒序遍历清理坏罐子
       for (int i = _jars.size() - 1; i >= 0; i--)
       {
           auto jar = _jars.at(i);

           // 检查指针有效性
           if (!jar || jar->getReferenceCount() == 0) {
               _jars.erase(i); continue;
           }
           if (jar->isDestroyed()) {
               _jars.erase(i); continue;
           }

           // 添加罐子顶部平台
           Rect topPlatform = jar->getTopPlatformBox();
           if (!topPlatform.equals(Rect::ZERO)) {
               dynamicGroundRects.push_back(topPlatform);
           }
       }
       _player->update(dt, dynamicGroundRects);
   }
    // ========================================
    // 2. 获取玩家位置并更新坐标显示
    // ========================================
   Vec2 playerPos = _player->getPosition();

   /* if (_coordLabel)
    {
        char coordText[100];
        sprintf(coordText, "Player: (%.0f, %.0f)", playerPos.x, playerPos.y);
        _coordLabel->setString(coordText);
    }*/

    // ========================================
    // 3. 相机立即跟随玩家
    // ========================================
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Size mapSize = map->getContentSize();
    float scaleValue = _gameLayer->getScale();

    float targetX = visibleSize.width * 0.5f - playerPos.x * scaleValue;
    float targetY = visibleSize.height * 0.5f - playerPos.y * scaleValue;

    float scaledMapWidth = mapSize.width * scaleValue;
    float scaledMapHeight = mapSize.height * scaleValue;

    float minX = -(scaledMapWidth - visibleSize.width);
    float maxX = 0.0f;
    float minY = -(scaledMapHeight - visibleSize.height);
    float maxY = 0.0f;

    if (scaledMapWidth > visibleSize.width) {
        targetX = std::max(minX, std::min(targetX, maxX));
    }
    else {
        targetX = (visibleSize.width - scaledMapWidth) * 0.5f;
    }

    if (scaledMapHeight > visibleSize.height) {
        targetY = std::max(minY, std::min(targetY, maxY));
    }
    else {
        targetY = (visibleSize.height - scaledMapHeight) * 0.5f;
    }

    _gameLayer->setPosition(targetX, targetY);

    // ============================================================
       // 3. 【优化】定义通用的怪物碰撞处理 Lambda
       // ============================================================
       // 这是一个局部函数，可以接受 Enemy*, Zombie*, Buzzer* 等任何有 standard 接口的指针
       // 这样就不用把相同的碰撞代码复制粘贴 3 遍了！
    auto handleCommonCollision = [&](auto* monster) {
        if (!monster) return;

        // 必须 Retain 防止在判定过程中被销毁
        monster->retain();

        Rect monsterBox = monster->getHitbox();
        if (!monsterBox.equals(Rect::ZERO))
        {
            bool isHit = false;
            // A. 攻击检测
            if (_player->isAttackPressed())
            {
                Rect attackBox = _player->getAttackHitbox();
                if (attackBox.intersectsRect(monsterBox))
                {
                    // 使用 tag 区分日志，方便调试
                    CCLOG("HIT! Player hit Monster (Tag: %d)", monster->getTag());

                    monster->takeDamage(1, _player->getPosition());
                    isHit = true;

                    if (_player->getAttackDir() == -1) {
                        _player->pogoJump();
                    }
                }
            }
            // B. 身体碰撞检测
            if (!isHit) {
                Rect playerBox = _player->getCollisionBox();
                if (playerBox.intersectsRect(monsterBox))
                {
                    if (!_player->isInvincible())
                    {
                        CCLOG("Player collided with Monster (Tag: %d)", monster->getTag());
                        _player->takeDamage(1, monster->getPosition(), _groundRects);
                        monster->onCollideWithPlayer(_player->getPosition());
                    }
                }
            }
        }
        monster->release();
        };

    // ============================================================
    // 4. 应用通用逻辑到各个怪物
    // ============================================================

    // --- Enemy (999) ---
    if (auto enemy = dynamic_cast<Enemy*>(_gameLayer->getChildByTag(999))) {
        // 先调用各自独特的 update (参数可能不同)
        // Enemy 的 update 需要 groundRects
        // 【注意】这里不要 retain，handleCommonCollision 里面会 retain
        enemy->update(dt);
        handleCommonCollision(enemy); // 传入 lambda 处理碰撞
    }

    // --- Zombie (998) ---
    if (auto zombie = dynamic_cast<Zombie*>(_gameLayer->getChildByTag(998))) {
        zombie->update(dt, playerPos, _groundRects);
        handleCommonCollision(zombie);
    }

    // --- Buzzer 1 (996) ---
    if (auto buzzer1 = dynamic_cast<Buzzer*>(_gameLayer->getChildByTag(996))) {
        // Buzzer 的 update 不需要 groundRects (根据你原代码)
        buzzer1->update(dt, playerPos);
        handleCommonCollision(buzzer1);
    }

    // --- Buzzer 2 (995) ---
    if (auto buzzer2 = dynamic_cast<Buzzer*>(_gameLayer->getChildByTag(995))) {
        buzzer2->update(dt, playerPos);
        handleCommonCollision(buzzer2);
    }
    // ========================================
    // 5. Spike 陷阱检测
    // ========================================
    auto spike = dynamic_cast<Spike*>(_gameLayer->getChildByTag(997));
    if (spike)
    {
        spike->update(dt, playerPos, _groundRects);

       /* if (_spikeDebugLabel)
        {
            char debugText[200];
            int state = (int)spike->getHitbox().equals(Rect::ZERO) ? -1 : 0;
            sprintf(debugText, "Spike:(%.0f,%.0f) State:%d Visible:%d",
                spike->getPosition().x, spike->getPosition().y,
                state, spike->isVisible());
            _spikeDebugLabel->setString(debugText);
        }*/

        Rect spikeBox = spike->getHitbox();
        if (!spikeBox.equals(Rect::ZERO))
        {
            Rect playerBox = _player->getCollisionBox();
            if (playerBox.intersectsRect(spikeBox))
            {
                if (!_player->isInvincible())
                {
                    CCLOG("Player hit by Spike!");
                    _player->takeDamage(1, spike->getPosition(), _groundRects);
                }
            }
        }
    }
   /* else
    {
        if (_spikeDebugLabel) {
            _spikeDebugLabel->setString("Spike: NOT FOUND (tag 997)");
            _spikeDebugLabel->setColor(Color3B::RED);
        }
    }*/
	

    // ========================================
    // 6. Jar 罐子碰撞检测
    // ========================================
   // 使用迭代器遍历，安全删除无效的罐子指针
    if (_currentLevel == 2 && !_jars.empty())
    {
        for (auto it = _jars.begin(); it != _jars.end(); )
        {
            auto jar = *it;

            // 1. 检查指针有效性
            if (!jar || jar->getReferenceCount() == 0)
            {
                it = _jars.erase(it); // 如果对象已失效，从列表中移除
                continue;
            }

            if (!jar || jar->isDestroyed())
            {
                it++;
                continue;
            }
            // A. 玩家攻击罐子
            if (_player->isAttackPressed())
            {
                Rect attackBox = _player->getAttackHitbox();
                Rect jarBox = jar->getCollisionBox();

                if (!jarBox.equals(Rect::ZERO) && attackBox.intersectsRect(jarBox))
                {
                    CCLOG("Player hit the jar!");
                    jar->takeDamage();
                    // 罐子也是可以下劈的
                    if (_player->getAttackDir() == -1) {
                        _player->pogoJump();
                    }

                    // ============================================================
                    // 【关键】检测 888 号罐子，打碎了才能生成复仇之魂
                    // ============================================================
                    if (jar->getTag() == 888&&jar->isDestroyed())
                    {
                        CCLOG("Special Jar Broken! Spawning Fireball at fixed position...");

                        auto fireball = Fireball::create("fireball/idle/fireball_1.png");
                        if (fireball)
                        {
                            fireball->setPosition(Vec2(5529.0f, 650.0f));

                            //  Tag (用于拾取)
                            fireball->setTag(987);

                            _gameLayer->addChild(fireball, 5);
                        }

                        // 标记已触发，防止重复生成
                        jar->setTag(-1);
                    }
                }
            }

            // B. 玩家与罐子的物理碰撞（像墙一样阻挡）
            if (!jar->isDestroyed())
            {
                Rect playerBox = _player->getCollisionBox();
                Rect jarBox = jar->getCollisionBox();

                if (!jarBox.equals(Rect::ZERO) && playerBox.intersectsRect(jarBox))
                {
                    // 简单的推出处理
                    float overlapLeft = (playerBox.getMaxX() - jarBox.getMinX());
                    float overlapRight = (jarBox.getMaxX() - playerBox.getMinX());

                    if (overlapLeft < overlapRight) {
                        _player->setPositionX(jarBox.getMinX() - playerBox.size.width / 2);
                    }
                    else {
                        _player->setPositionX(jarBox.getMaxX() + playerBox.size.width / 2);
                    }
                    _player->setVelocityX(0);
                }
            }
            it++;
        }
    }

    // ========================================
    // 8. 复仇之魂拾取逻辑 (Tag 987)
    // ========================================
    auto skillItemNode = _gameLayer->getChildByTag(987);
    if (skillItemNode)
    {
        // 安全转换
        auto skillItem = dynamic_cast<Fireball*>(skillItemNode);
        if (skillItem && _player)
        {
            Rect playerBox = _player->getCollisionBox();
            Rect itemBox = skillItem->getBoundingBox();

            if (playerBox.intersectsRect(itemBox))
            {
                CCLOG("INTERACTION: Acquired Vengeful Spirit!");

                // 1. 解锁技能
                _player->unlockFireball();

                // 2. 播放特效 (变大消失)
                skillItem->stopAllActions();
                skillItem->runAction(Sequence::create(
                    ScaleTo::create(0.2f, 2.0f),
                    FadeOut::create(0.2f),
                    CallFunc::create([skillItem]() { skillItem->removeFromParent(); }),
                    nullptr
                ));

                // 3. 立即去标签，防止重复触发
                skillItem->setTag(-1);
            }
        }
    }

    // ========================================
    // 9. Boss 战斗逻辑 (Level 3)
    // ========================================
    if (_currentLevel == 3)
    {
        updateBossInteraction(dt); // Boss 本体
        updateBossProjectiles(dt); // Boss 弹幕
    }

    // ========================================
    // 10. FKFireball 碰撞检测 (Level 3)
    // ========================================
    if (_currentLevel == 3)
    {
        auto children = _gameLayer->getChildren();
        for (auto child : children)
        {
            auto fireball = dynamic_cast<FKFireball*>(child);
            if (fireball)
            {
                fireball->update(dt, _groundRects);
                Rect fireballBox = fireball->getCollisionBox();
                if (!fireballBox.equals(Rect::ZERO))
                {
                    Rect playerBox = _player->getCollisionBox();
                    if (playerBox.intersectsRect(fireballBox))
                    {
                        if (!_player->isInvincible())
                        {
                            CCLOG("Player hit by FKFireball!");
                            _player->takeDamage(1, fireball->getPosition(), _groundRects);
                            fireball->removeFromParent(); // 火球碰到玩家后消失
                        }
                    }
                }
            }
            else if (auto shockwave = dynamic_cast<FKShockwave*>(child))
            {
                shockwave->update(dt, _groundRects);
                Rect swBox = shockwave->getCollisionBox();
                if (!swBox.equals(Rect::ZERO))
                {
                    Rect playerBox = _player->getCollisionBox();
                    if (playerBox.intersectsRect(swBox))
                    {
                        if (!_player->isInvincible())
                        {
                            CCLOG("Player hit by FKShockwave!");
                            _player->takeDamage(1, shockwave->getPosition(), _groundRects);
                            shockwave->removeFromParent();
                        }
                    }
                }
            }
        }
    }

    // ============================================================
    // 11.主角火球 vs Boss 碰撞检测
    // ============================================================
    if (_currentLevel == 3 && _boss && _bossTriggered)
    {
        // 遍历所有子节点找到主角的火球
        auto children = _gameLayer->getChildren();
        for (auto child : children)
        {
            // 1. 筛选：必须是 Fireball 类，且 Tag 为 5000 (Player.cpp里定义的)
            auto pFireball = dynamic_cast<Fireball*>(child);
            if (pFireball && pFireball->getTag() == 5000)
            {
                // 2. 获取 Boss 的身体判定框
                Rect bossBox = _boss->getBodyHitbox();

                // 3. 检测碰撞
                if (!bossBox.equals(Rect::ZERO) && pFireball->getHitbox().intersectsRect(bossBox))
                {
                    CCLOG("HIT! Vengeful Spirit hit the Boss!");

                    // A. Boss 扣血 (法术伤害通常比平砍高，设为 3 或者 10)
                    _boss->takeDamage(3);

                    // B. 【关键】火球消失
                    pFireball->removeFromParent();

                    // C. (可选) 可以在这里加一个击中特效
                    // auto hitEffect = HitEffect::create(...);
                    // _gameLayer->addChild(hitEffect);
                }
            }
        }
    }

    // ========================================
    // 12.梦之钉 碰撞检测
    // ========================================
    if (_player->isDreamNailActive())
    {
        bool hasHit = false; // 防止一帧内多次判定

        // 定义一个通用的 Lambda，接受任何 GameEntity (Enemy, Buzzer, Jar)
        auto handleDreamHit = [&](GameEntity* entity) {
            // 1. 基础校验：存在、未命中其他、碰撞框相交、且实体逻辑有效(未销毁)
            if (!hasHit && entity && entity->isValidEntity())
            {
                // 使用多态获取碰撞箱 (Jar 和 Enemy 的 getHitbox 实现不同，但接口一致)
                if (_player->getDreamNailHitbox().intersectsRect(entity->getHitbox()))
                {
                    // 2. 【核心】调用实体内部的梦语逻辑
                    // 它会自动读取 setDreamThought 设置的文本并弹窗
                    entity->onDreamNailHit();

                    // 3. 关闭主角的梦钉判定框，防止连续触发
                    _player->setDreamNailActive(false);
                    hasHit = true;

                    CCLOG("Dream Nail hit entity Tag: %d", entity->getTag());
                }
            }
            };

        // -------------------------------------------------
        // A. 检测怪物 (Enemy, Zombie, Buzzer)
        // -------------------------------------------------
        // 因为它们都继承自 GameEntity，所以可以直接转换
        handleDreamHit(dynamic_cast<GameEntity*>(_gameLayer->getChildByTag(999))); // Enemy
        handleDreamHit(dynamic_cast<GameEntity*>(_gameLayer->getChildByTag(998))); // Zombie
        handleDreamHit(dynamic_cast<GameEntity*>(_gameLayer->getChildByTag(996))); // Buzzer 1
        handleDreamHit(dynamic_cast<GameEntity*>(_gameLayer->getChildByTag(995))); // Buzzer 2

        // -------------------------------------------------
        // B. 检测罐子 (Jars)
        // -------------------------------------------------
        // Jar 也继承自 GameEntity，所以逻辑完全通用！
        if (!hasHit && _currentLevel == 2 && !_jars.empty())
        {
            for (auto jar : _jars)
            {
                handleDreamHit(jar);
                if (hasHit) break; // 如果命中一个罐子，就跳出循环
            }
        }
    }
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}

// ========================================
// 加载地图的通用方法
// ========================================
void HelloWorld::loadMap(const std::string& mapPath)
{
    // 【修复建议】在移除旧地图之前，首先清理依赖于它的对象
    if (_currentLevel == 3)
    {
        // 清理 Level 2 的敌人和对象
        // 【修复】不再遍历 _jars 向量，因为它可能包含悬空指针。
        // 改为通过 tag 从 _gameLayer 中安全地查找并移除。
        for (int i = 0; i < 3; ++i)
        {
            auto jarNode = _gameLayer->getChildByTag(990 - i);
            if (jarNode)
            {
                jarNode->removeFromParent();
            }
        }

        // 【修复】立即清空向量，防止悬空指针问题
        _jars.clear();

        // 清除 Fireball
        auto oldFireball = _gameLayer->getChildByTag(987);
        if (oldFireball) oldFireball->removeFromParent();
    }

    // 1. 清除旧地图
    auto oldMap = _gameLayer->getChildByTag(123);
    if (oldMap) oldMap->removeFromParent();

    // 2. 清除旧的碰撞框绘制节点
    auto oldDrawNode = dynamic_cast<DrawNode*>(_gameLayer->getChildByTag(1000));
    if (oldDrawNode) oldDrawNode->removeFromParent();

    // 3. 清除旧的敌人
    if (_currentLevel == 2)
    {
        // 清理 Level 1 的敌人
        auto enemy = _gameLayer->getChildByTag(999);
        if (enemy) enemy->removeFromParent();

        auto zombie = _gameLayer->getChildByTag(998);
        if (zombie) zombie->removeFromParent();

        auto spike = _gameLayer->getChildByTag(997);
        if (spike) spike->removeFromParent();

        auto buzzer1 = _gameLayer->getChildByTag(996);
        if (buzzer1) buzzer1->removeFromParent();

        auto buzzer2 = _gameLayer->getChildByTag(995);
        if (buzzer2) buzzer2->removeFromParent();
    }

    // 【新增】清理旧的 Boss
    if (_boss)
    {
        _boss->removeFromParent();
        _boss = nullptr;
        _bossTriggered = false;
    }

    CCLOG("DEBUG_STEP_1: Starting TMXTiledMap::create with path: %s", mapPath.c_str());
   
    // 4. 加载新地图
    auto map = TMXTiledMap::create(mapPath);
    if (map == nullptr) {
        CCLOG("Error: Failed to load %s", mapPath.c_str());
        return;
    }
    CCLOG("DEBUG_STEP_2: Map created successfully. Setting tag...");

    map->setAnchorPoint(Vec2(0, 0));
    map->setPosition(Vec2(0, 0));
    map->setTag(123);
    _gameLayer->addChild(map, -99);

    CCLOG("DEBUG_STEP_3: Parsing collisions...");

    // 5. 解析碰撞数据
    this->parseMapCollisions(map);
    CCLOG("DEBUG_STEP_4: Collisions parsed. GroundRects size: %d", (int)_groundRects.size());
    // ============================================================
    // 【修改】Level 3 偏移量统一管理
    // 定义一个 mapOffset 变量，同时应用于背景图和碰撞框
    // ============================================================
    Vec2 mapOffset = Vec2::ZERO;
    CCLOG("DEBUG_STEP_5: Creating Boss...");

    if (_currentLevel == 3)
    {
        // 这里设置你想要的偏移量，例如 (0, 300)
        mapOffset = Vec2(300, 250);

        // 【关键】修正碰撞框位置，使其跟随偏移量移动
        for (auto& rect : _groundRects)
        {
            rect.origin += mapOffset;
        }
    }

    // 6. 绘制调试碰撞框 (现在会绘制修正后的位置)
    // auto drawNode = DrawNode::create();
    // drawNode->setTag(1000);
    // _gameLayer->addChild(drawNode, 999);

    // for (const auto& rect : _groundRects)
    // {
    //     drawNode->drawRect(rect.origin, rect.origin + rect.size, Color4F::RED);
    // }

    CCLOG("========== Map Loaded: %s ==========", mapPath.c_str());

    // 7. 根据关卡创建特定的敌人和对象
    if (_currentLevel == 2)
    {
        _jars.clear();

        Jar::setupPuzzleJars(_gameLayer, _jars);

        // ============================================================
        // 【新增】进入 Level 2 时的解谜提示
        // ============================================================
        auto visibleSize = Director::getInstance()->getVisibleSize();

        // 创建对话框
        auto hintDialog = DreamDialogue::create("Listen to the dream...Three voices... Only one speaks the truth...Save that one to hold the flame...");

        if (hintDialog)
        {
            // 设置位置：屏幕上方居中 (X居中, Y靠上)
            // 注意：因为是直接加到 Scene (this) 上，所以不受 Camera 移动影响，是 UI 坐标
            hintDialog->setPosition(Vec2(visibleSize.width / 2, visibleSize.height-300));

            // 加到最上层 (ZOrder 200)
            this->addChild(hintDialog, 200);

            // 显示
            hintDialog->show();

            CCLOG("Level 2 Hint displayed.");
        }

    }
    else if (_currentLevel == 3)
    {
        // ============================================================
        // Level 3 特殊处理：手动加载背景图
        // ============================================================
        auto bg = Sprite::create("maps/GameAsset/fight.png"); // 确保路径对应 Resources/maps/GameAsset/fight.png
        if (bg)
        {
            bg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);

            // 【用户请求】使用上面定义的 mapOffset 来设置背景图位置
            // 这样背景图和碰撞框就同步移动了！
            bg->setPosition(mapOffset);

            // 添加到 map 节点的最底层 (-1)，这样它永远在 map 的其他元素（如碰撞）后面
            map->addChild(bg, -1);

            // 强制同步地图大小为背景图大小
            map->setContentSize(bg->getContentSize());

            CCLOG("Success: Level 3 background loaded manually.");
        }
        else
        {
            CCLOG("Error: Failed to load Level 3 background 'maps/GameAsset/fight.png'");
        }

        // ============================================================
        // 【新增】创建 Boss
        // ============================================================
        // 地板 Y 坐标是 856（从 level3.tmx collision 可以看到）
        // Boss 应该站在地板上，所以 Y = 856
        float groundY = 856.0f + mapOffset.y;
        _boss = Boss::create(Vec2(1650 + mapOffset.x, groundY + 200));  // +200 让它从空中掉落
       
        CCLOG("DEBUG_STEP_6: Boss create() returned.");

        if (_boss)
        {
            _boss->setTag(980);  // Boss tag
            _gameLayer->addChild(_boss, 6);
            _bossTriggered = false;

            // 【新增】设置火球回调
            _boss->setFireballCallback([this, mapOffset](const Vec2& pos) {
                auto fireball = FKFireball::create("boss/rampageAttack/fk-fireball.png");
                if (fireball)
                {
                    // 【修复】将 Boss 局部随机坐标转换为世界坐标
                    fireball->setPosition(Vec2(pos.x, pos.y));
                    _gameLayer->addChild(fireball, 7); // 确保在 Boss 上面
                }
                });
            _boss->setShockwaveCallback([this](const Vec2& pos, float dir) {
                auto shockwave = FKShockwave::create("boss/shockwaveAttack/fk-shockwave.png", dir);
                if (shockwave)
                {
                    shockwave->setPosition(pos);
                    _gameLayer->addChild(shockwave, 7);
                }
                });

            CCLOG("Boss created at (%.0f, %.0f) for falling", 1650 + mapOffset.x, groundY + 200);
        }
        else
        {
            CCLOG("Error: Failed to create Boss!");
        }

        // 可以在这里添加 Level 3 的敌人
        CCLOG("Level 3 loaded - Ready for battle!");
    }

// ============================================================
    // 【新增】音乐切换逻辑
    // ============================================================
auto audio = CocosDenshion::SimpleAudioEngine::getInstance();

if (_currentLevel == 3)
{
    // 进入 Boss 关：切换到战斗音乐
    audio->stopBackgroundMusic();
    audio->playBackgroundMusic(Config::Audio::BGM_BOSS, true); // true = 循环播放
    CCLOG("Music: Switched to BOSS BGM");
}
else
{
   
}
}
// ========================================
// 切换到Level2的方法
// ========================================
void HelloWorld::switchToLevel2()
{
    if (_isTransitioning || _currentLevel == 2) return;

    _isTransitioning = true;
    CCLOG("========== Switching to Level 2 ==========");

    auto blackLayer = LayerColor::create(Color4B::BLACK);
    blackLayer->setOpacity(0);
    this->addChild(blackLayer, 999);

    auto fadeIn = FadeTo::create(0.5f, 255);

    auto switchMap = CallFunc::create([this]() {
        _currentLevel = 2;
        loadMap("maps/level2.tmx");

        if (_player)
        {
            _player->setPosition(Vec2(400, 1300));
            _player->setVelocityX(0);
        }
        });

    auto delay = DelayTime::create(0.3f);
    auto fadeOut = FadeTo::create(0.5f, 0);

    auto cleanup = CallFunc::create([this, blackLayer]() {
        blackLayer->removeFromParent();
        _isTransitioning = false;
        CCLOG("========== Level 2 loaded successfully ==========");
        });

    blackLayer->runAction(Sequence::create(fadeIn, switchMap, delay, fadeOut, cleanup, nullptr));
}

// ========================================
// 切换到Level3的方法
// ========================================
void HelloWorld::switchToLevel3()
{
    if (_isTransitioning || _currentLevel == 3) return;

    _isTransitioning = true;
    CCLOG("========== Switching to Level 3 ==========");

    auto blackLayer = LayerColor::create(Color4B::BLACK);
    blackLayer->setOpacity(0);
    this->addChild(blackLayer, 999);

    auto fadeIn = FadeTo::create(0.5f, 255);

    auto switchMap = CallFunc::create([this]() {
        _currentLevel = 3;
        loadMap("maps/level3.tmx");

        if (_player)
        {
            _player->setPosition(Vec2(500, 1300));
            _player->setVelocityX(0);
        }
        });

    auto delay = DelayTime::create(0.3f);
    auto fadeOut = FadeTo::create(0.5f, 0);

    auto cleanup = CallFunc::create([this, blackLayer]() {
        blackLayer->removeFromParent();
        _isTransitioning = false;
        CCLOG("========== Level 3 loaded successfully ==========");
        });

    blackLayer->runAction(Sequence::create(fadeIn, switchMap, delay, fadeOut, cleanup, nullptr));
}

// ========================================
// 切换到Level1的方法
// ========================================
void HelloWorld::switchToLevel1()
{
    if (_isTransitioning || _currentLevel == 1) return;
    _isTransitioning = true;
    CCLOG("========== Switching to Level 1 ==========");
    auto blackLayer = LayerColor::create(Color4B::BLACK);
    blackLayer->setOpacity(0);
    this->addChild(blackLayer, 999);
    auto fadeIn = FadeTo::create(0.5f, 255);
    auto switchMap = CallFunc::create([this]() {
        _currentLevel = 1;
        loadMap("maps/level1.tmx");
        if (_player)
        {
            // 切回level1时主角出现在最左端
            _player->setPosition(Vec2(435, 1300));
            _player->setVelocityX(0);
        }
        });
    auto delay = DelayTime::create(0.3f);
    auto fadeOut = FadeTo::create(0.5f, 0);
    auto cleanup = CallFunc::create([this, blackLayer]() {
        blackLayer->removeFromParent();
        _isTransitioning = false;
        CCLOG("========== Level 1 loaded successfully ==========");
        });
    blackLayer->runAction(Sequence::create(fadeIn, switchMap, delay, fadeOut, cleanup, nullptr));
}

// ========================================
// 从右侧切换回Level2的方法 (针对Level 3)
// ========================================
void HelloWorld::switchToLevel2FromRight()
{
    if (_isTransitioning || _currentLevel == 2) return;
    _isTransitioning = true;
    CCLOG("========== Switching to Level 2 (from right) ==========");
    auto blackLayer = LayerColor::create(Color4B::BLACK);
    blackLayer->setOpacity(0);
    this->addChild(blackLayer, 999);
    auto fadeIn = FadeTo::create(0.5f, 255);
    auto switchMap = CallFunc::create([this]() {
        _currentLevel = 2;
        loadMap("maps/level2.tmx");
        if (_player)
        {
            // 切回level2时主角出现在最右端
            _player->setPosition(Vec2(6200, 1300));
            _player->setVelocityX(0);
        }
        });
    auto delay = DelayTime::create(0.3f);
    auto fadeOut = FadeTo::create(0.5f, 0);
    auto cleanup = CallFunc::create([this, blackLayer]() {
        blackLayer->removeFromParent();
        _isTransitioning = false;
        CCLOG("========== Level 2 loaded successfully (from right) ==========");
        });
    blackLayer->runAction(Sequence::create(fadeIn, switchMap, delay, fadeOut, cleanup, nullptr));
}

// ==========================================================
// Boss 逻辑实现 (独立于 update)
// ==========================================================

void HelloWorld::updateBossInteraction(float dt)
{
    if (!_boss) return;

    Vec2 playerPos = _player->getPosition();

    // 1. 触发逻辑
    if (!_bossTriggered && playerPos.x >= 1000.0f) {
        _bossTriggered = true;
        CCLOG("========== BOSS TRIGGERED ==========");
    }
    if (!_bossTriggered) return;

    // 2. 更新 AI
    _boss->updateBoss(dt, playerPos, _groundRects);

    // 3. 碰撞检测
    _boss->retain(); // 保命

    // Body 碰撞
    Rect bossBodyBox = _boss->getBodyHitbox();
    if (!bossBodyBox.equals(Rect::ZERO))
    {
        bool isBossHit = false;

        // A. 玩家平砍
        if (_player->isAttackPressed()) {
            if (_player->getAttackHitbox().intersectsRect(bossBodyBox)) {
                CCLOG("HIT! Player hit the Boss!");
                _boss->takeDamage(1);
                isBossHit = true;
                if (_player->getAttackDir() == -1) _player->pogoJump();

                // 受击回魂
                static int bossHitCombo = 0;
                if (++bossHitCombo >= 3) {
                    bossHitCombo = 0;
                    if (_player->getStats()->getSoul() < _player->getStats()->getMaxSoul())
                        _player->gainSoul(1);
                }
            }
        }

        // B. 玩家法术 (复仇之魂)
        auto children = _gameLayer->getChildren();
        for (auto child : children) {
            if (auto pFireball = dynamic_cast<Fireball*>(child)) {
                if (pFireball->getTag() == 5000 && pFireball->getHitbox().intersectsRect(bossBodyBox)) {
                    _boss->takeDamage(3);
                    pFireball->removeFromParent();
                }
            }
        }

        // C. 撞人
        if (!isBossHit) {
            if (_player->getCollisionBox().intersectsRect(bossBodyBox) && !_player->isInvincible()) {
                _player->takeDamage(1, _boss->getPosition(), _groundRects);
            }
        }
    }

    // 4. 锤子碰撞
    Rect bossHammerBox = _boss->getHammerHitbox();
    if (!bossHammerBox.equals(Rect::ZERO)) {
        if (_player->getCollisionBox().intersectsRect(bossHammerBox) && !_player->isInvincible()) {
            _player->takeDamage(1, _boss->getPosition(), _groundRects);
        }
    }

    _boss->release();
}

void HelloWorld::updateBossProjectiles(float dt)
{
    auto children = _gameLayer->getChildren();
    for (auto child : children)
    {
        // 1. FKFireball
        if (auto fireball = dynamic_cast<FKFireball*>(child)) {
            fireball->update(dt, _groundRects);
            if (!fireball->getCollisionBox().equals(Rect::ZERO) &&
                _player->getCollisionBox().intersectsRect(fireball->getCollisionBox()) &&
                !_player->isInvincible())
            {
                _player->takeDamage(1, fireball->getPosition(), _groundRects);
                fireball->removeFromParent();
            }
            continue;
        }

        // 2. FKShockwave
        if (auto shockwave = dynamic_cast<FKShockwave*>(child)) {
            shockwave->update(dt, _groundRects);
            if (!shockwave->getCollisionBox().equals(Rect::ZERO) &&
                _player->getCollisionBox().intersectsRect(shockwave->getCollisionBox()) &&
                !_player->isInvincible())
            {
                _player->takeDamage(1, shockwave->getPosition(), _groundRects);
                shockwave->removeFromParent();
            }
        }
    }
}