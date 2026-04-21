#include "KeyBindingScene.h"
#include "HelloWorldScene.h"
#include "ui/UIButton.h"

// ========================================
// KeyBindingManager 实现
// ========================================

KeyBindingManager* KeyBindingManager::s_instance = nullptr;

KeyBindingManager* KeyBindingManager::getInstance()
{
    if (!s_instance)
    {
        s_instance = new KeyBindingManager();
    }
    return s_instance;
}

KeyBindingManager::KeyBindingManager()
{
    initDefaultKeys();
    // 自动加载已保存的配置
    loadFromFile();
}

void KeyBindingManager::initDefaultKeys()
{
    // 默认键位配置
    _keyBindings[Action::MOVE_UP] = EventKeyboard::KeyCode::KEY_UP_ARROW;
    _keyBindings[Action::MOVE_DOWN] = EventKeyboard::KeyCode::KEY_DOWN_ARROW;
    _keyBindings[Action::MOVE_LEFT] = EventKeyboard::KeyCode::KEY_LEFT_ARROW;
    _keyBindings[Action::MOVE_RIGHT] = EventKeyboard::KeyCode::KEY_RIGHT_ARROW;
    _keyBindings[Action::JUMP] = EventKeyboard::KeyCode::KEY_Z;
    _keyBindings[Action::ATTACK] = EventKeyboard::KeyCode::KEY_X;
    _keyBindings[Action::FOCUS] = EventKeyboard::KeyCode::KEY_A;
    _keyBindings[Action::DASH] = EventKeyboard::KeyCode::KEY_D;
    _keyBindings[Action::CAST_SPELL] = EventKeyboard::KeyCode::KEY_S;
    _keyBindings[Action::DREAM_NAIL] = EventKeyboard::KeyCode::KEY_E;
    _keyBindings[Action::PAUSE] = EventKeyboard::KeyCode::KEY_SPACE;
    _keyBindings[Action::CONFIRM] = EventKeyboard::KeyCode::KEY_C;
}

EventKeyboard::KeyCode KeyBindingManager::getKeyForAction(Action action) const
{
    auto it = _keyBindings.find(action);
    if (it != _keyBindings.end())
    {
        return it->second;
    }
    return EventKeyboard::KeyCode::KEY_NONE;
}

void KeyBindingManager::setKeyForAction(Action action, EventKeyboard::KeyCode key)
{
    _keyBindings[action] = key;
}

std::string KeyBindingManager::getKeyName(EventKeyboard::KeyCode key)
{
    switch (key)
    {
    // Use text instead of Unicode arrows to avoid rendering issues
    case EventKeyboard::KeyCode::KEY_UP_ARROW: return "UP";
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW: return "DOWN";
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW: return "LEFT";
    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW: return "RIGHT";
    case EventKeyboard::KeyCode::KEY_Z: return "Z";
    case EventKeyboard::KeyCode::KEY_X: return "X";
    case EventKeyboard::KeyCode::KEY_C: return "C";
    case EventKeyboard::KeyCode::KEY_A: return "A";
    case EventKeyboard::KeyCode::KEY_S: return "S";
    case EventKeyboard::KeyCode::KEY_D: return "D";
    case EventKeyboard::KeyCode::KEY_W: return "W";
    case EventKeyboard::KeyCode::KEY_Q: return "Q";
    case EventKeyboard::KeyCode::KEY_E: return "E";
    case EventKeyboard::KeyCode::KEY_SPACE: return "SPACE";
    case EventKeyboard::KeyCode::KEY_SHIFT: return "SHIFT";
    case EventKeyboard::KeyCode::KEY_CTRL: return "CTRL";
    case EventKeyboard::KeyCode::KEY_ALT: return "ALT";
    case EventKeyboard::KeyCode::KEY_TAB: return "TAB";
    case EventKeyboard::KeyCode::KEY_ENTER: return "ENTER";
    case EventKeyboard::KeyCode::KEY_ESCAPE: return "ESC";
    case EventKeyboard::KeyCode::KEY_BACKSPACE: return "BACKSPACE";
    
    // Number keys
    case EventKeyboard::KeyCode::KEY_0: return "0";
    case EventKeyboard::KeyCode::KEY_1: return "1";
    case EventKeyboard::KeyCode::KEY_2: return "2";
    case EventKeyboard::KeyCode::KEY_3: return "3";
    case EventKeyboard::KeyCode::KEY_4: return "4";
    case EventKeyboard::KeyCode::KEY_5: return "5";
    case EventKeyboard::KeyCode::KEY_6: return "6";
    case EventKeyboard::KeyCode::KEY_7: return "7";
    case EventKeyboard::KeyCode::KEY_8: return "8";
    case EventKeyboard::KeyCode::KEY_9: return "9";
    
    // Letter keys (A-Z already covered above, add the rest)
    case EventKeyboard::KeyCode::KEY_B: return "B";
    case EventKeyboard::KeyCode::KEY_F: return "F";
    case EventKeyboard::KeyCode::KEY_G: return "G";
    case EventKeyboard::KeyCode::KEY_H: return "H";
    case EventKeyboard::KeyCode::KEY_I: return "I";
    case EventKeyboard::KeyCode::KEY_J: return "J";
    case EventKeyboard::KeyCode::KEY_K: return "K";
    case EventKeyboard::KeyCode::KEY_L: return "L";
    case EventKeyboard::KeyCode::KEY_M: return "M";
    case EventKeyboard::KeyCode::KEY_N: return "N";
    case EventKeyboard::KeyCode::KEY_O: return "O";
    case EventKeyboard::KeyCode::KEY_P: return "P";
    case EventKeyboard::KeyCode::KEY_R: return "R";
    case EventKeyboard::KeyCode::KEY_T: return "T";
    case EventKeyboard::KeyCode::KEY_U: return "U";
    case EventKeyboard::KeyCode::KEY_V: return "V";
    case EventKeyboard::KeyCode::KEY_Y: return "Y";
    
    default: return "?";
    }
}

std::string KeyBindingManager::getActionName(Action action)
{
    switch (action)
    {
    case Action::MOVE_UP: return "MOVE UP";
    case Action::MOVE_DOWN: return "MOVE DOWN";
    case Action::MOVE_LEFT: return "MOVE LEFT";
    case Action::MOVE_RIGHT: return "MOVE RIGHT";
    case Action::JUMP: return "JUMP";
    case Action::ATTACK: return "ATTACK";
    case Action::FOCUS: return "FOCUS";
    case Action::DASH: return "DASH";
    case Action::CAST_SPELL: return "CAST SPELL";
    case Action::DREAM_NAIL: return "DREAM NAIL";
    case Action::PAUSE: return "PAUSE GAME";
    case Action::CONFIRM: return "START / CONTINUE";
    default: return "Unknown";
    }
}

void KeyBindingManager::saveToFile()
{
    CCLOG("========== Saving Key Bindings ==========");
    
    auto userDefault = UserDefault::getInstance();
    
    // 保存每个动作对应的按键
    for (const auto& binding : _keyBindings)
    {
        std::string key = "KeyBinding_" + std::to_string((int)binding.first);
        int keyCode = (int)binding.second;
        
        userDefault->setIntegerForKey(key.c_str(), keyCode);
        
        CCLOG("  Saved: %s -> KeyCode %d", 
              getActionName(binding.first).c_str(), 
              keyCode);
    }
    
    // 立即刷新到文件
    userDefault->flush();
    
    CCLOG("Key bindings saved successfully!");
    CCLOG("========================================");
}

void KeyBindingManager::loadFromFile()
{
    CCLOG("========== Loading Key Bindings ==========");
    
    auto userDefault = UserDefault::getInstance();
    bool hasCustomBindings = false;
    
    // 尝试加载每个动作的按键配置
    for (auto& binding : _keyBindings)
    {
        std::string key = "KeyBinding_" + std::to_string((int)binding.first);
        
        // 检查是否存在保存的值（使用一个不可能的键码作为默认值）
        int savedKeyCode = userDefault->getIntegerForKey(key.c_str(), -1);
        
        if (savedKeyCode != -1)
        {
            binding.second = (EventKeyboard::KeyCode)savedKeyCode;
            hasCustomBindings = true;
            
            CCLOG("  Loaded: %s -> KeyCode %d (%s)", 
                  getActionName(binding.first).c_str(), 
                  savedKeyCode,
                  getKeyName(binding.second).c_str());
        }
        else
        {
            CCLOG("  No saved binding for %s, using default: %s", 
                  getActionName(binding.first).c_str(),
                  getKeyName(binding.second).c_str());
        }
    }
    
    if (hasCustomBindings)
    {
        CCLOG("Custom key bindings loaded successfully!");
    }
    else
    {
        CCLOG("No saved bindings found, using defaults.");
    }
    
    CCLOG("=========================================");
}

void KeyBindingManager::resetToDefault()
{
    initDefaultKeys();
    
    // 保存默认配置到文件
    saveToFile();
    
    CCLOG("Key bindings reset to default and saved");
}

// ========================================
// KeyBindingScene 实现
// ========================================

Scene* KeyBindingScene::createScene()
{
    return KeyBindingScene::create();
}

bool KeyBindingScene::init()
{
    if (!Scene::init())
    {
        return false;
    }
    
    _currentSelection = 0;
    _isWaitingForKey = false;
    _isPauseMode = false;// 默认为开始界面模式

    // 定义动作顺序（左右列）
    _actionOrder = {
        KeyBindingManager::Action::MOVE_UP,
        KeyBindingManager::Action::MOVE_DOWN,
        KeyBindingManager::Action::MOVE_LEFT,
        KeyBindingManager::Action::MOVE_RIGHT,
        KeyBindingManager::Action::JUMP,
        KeyBindingManager::Action::ATTACK,
        KeyBindingManager::Action::FOCUS,
        KeyBindingManager::Action::DASH,
        KeyBindingManager::Action::CAST_SPELL,
        KeyBindingManager::Action::DREAM_NAIL,
        KeyBindingManager::Action::PAUSE,
        KeyBindingManager::Action::CONFIRM
    };
    
    createUI();
    setupKeyboardListener();
    
    return true;
}

void KeyBindingScene::createUI()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    // 深色背景 - 模仿空洞骑士的深蓝黑色调
    auto bgLayer = LayerColor::create(Color4B(15, 15, 25, 255));
    this->addChild(bgLayer, -1);
    
    createTitle();
    createActionList();
    createButtons();
    createInstructions();
}

void KeyBindingScene::createTitle()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    // 标题: "KEYBOARD SETTINGS"
    _titleLabel = Label::createWithSystemFont("KEYBOARD SETTINGS", "Arial", 56);
    _titleLabel->setColor(Color3B(220, 220, 240));
    _titleLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    this->addChild(_titleLabel, 10);
    
    // 装饰性分隔线 - 更细更优雅
    auto line1 = DrawNode::create();
    line1->drawLine(Vec2(visibleSize.width / 2 - 300, visibleSize.height - 140),
                    Vec2(visibleSize.width / 2 + 300, visibleSize.height - 140),
                    Color4F(0.5f, 0.5f, 0.6f, 0.8f));
    this->addChild(line1, 9);
}

void KeyBindingScene::createActionList()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    float startY = visibleSize.height - 220;
    float itemHeight = 70;  // 增加行间距
    float leftX = visibleSize.width / 2 - 280;
    float rightX = visibleSize.width / 2 + 180;
    
    auto kbm = KeyBindingManager::getInstance();
    
    // 创建左右两列
    for (size_t i = 0; i < _actionOrder.size(); i++)
    {
        auto action = _actionOrder[i];
        
        // 左侧：动作名称（英文，全大写）
        auto actionLabel = Label::createWithSystemFont(
            kbm->getActionName(action),
            "Arial",
            32
        );
        actionLabel->setAnchorPoint(Vec2(1.0f, 0.5f));
        actionLabel->setPosition(Vec2(leftX, startY - i * itemHeight));
        actionLabel->setColor(Color3B(180, 180, 200));
        this->addChild(actionLabel, 5);
        _actionLabels.push_back(actionLabel);
        
        // 右侧：按键显示 - 使用框体包裹
        auto keyLabel = Label::createWithSystemFont(
            kbm->getKeyName(kbm->getKeyForAction(action)),
            "Arial",
            30
        );
        keyLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
        keyLabel->setPosition(Vec2(rightX, startY - i * itemHeight));
        keyLabel->setColor(Color3B(240, 240, 255));
        this->addChild(keyLabel, 6);
        _keyLabels.push_back(keyLabel);
        
        // 为按键添加背景框（类似空洞骑士的按键样式）
        auto keyBg = Sprite::create();
        keyBg->setTextureRect(Rect(0, 0, 100, 50));
        keyBg->setColor(Color3B(40, 40, 60));
        keyBg->setOpacity(180);
        keyBg->setPosition(Vec2(rightX, startY - i * itemHeight));
        this->addChild(keyBg, 5);
    }
    
    // 创建选择光标 - 更醒目的高亮效果
    _selector = Sprite::create();
    _selector->setTextureRect(Rect(0, 0, 600, 60));
    _selector->setColor(Color3B(80, 120, 180));
    _selector->setOpacity(120);
    _selector->setAnchorPoint(Vec2(0.5f, 0.5f));
    this->addChild(_selector, 4);
    
    updateSelector();
}

void KeyBindingScene::createButtons()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    
    float buttonY = 140;
    
    // "APPLY & START" 按钮
    auto startLabel = Label::createWithSystemFont("APPLY & START", "Arial", 36);
    startLabel->setColor(Color3B(200, 240, 200));
    startLabel->setPosition(Vec2(visibleSize.width / 2 - 200, buttonY));
    this->addChild(startLabel, 10);
    
    // 按钮背景
    auto startBg = Sprite::create();
    startBg->setTextureRect(Rect(0, 0, 280, 60));
    startBg->setColor(Color3B(40, 60, 40));
    startBg->setOpacity(150);
    startBg->setPosition(Vec2(visibleSize.width / 2 - 200, buttonY));
    this->addChild(startBg, 9);
    
    // "RESET TO DEFAULT" 按钮
    auto resetLabel = Label::createWithSystemFont("RESET DEFAULT", "Arial", 36);
    resetLabel->setColor(Color3B(240, 200, 200));
    resetLabel->setPosition(Vec2(visibleSize.width / 2 + 200, buttonY));
    this->addChild(resetLabel, 10);
    
    // 按钮背景
    auto resetBg = Sprite::create();
    resetBg->setTextureRect(Rect(0, 0, 280, 60));
    resetBg->setColor(Color3B(60, 40, 40));
    resetBg->setOpacity(150);
    resetBg->setPosition(Vec2(visibleSize.width / 2 + 200, buttonY));
    this->addChild(resetBg, 9);
}

void KeyBindingScene::createInstructions()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    _hintLabel1 = Label::createWithSystemFont(
        "UP/DOWN: Select    ENTER: Change Key    R: Reset",
        "Arial", 48
    );
    _hintLabel1->setColor(Color3B(160, 160, 180));
    _hintLabel1->setPosition(Vec2(visibleSize.width / 2, 280));
    this->addChild(_hintLabel1, 10);

    // 获取当前确认键的名字
    auto kbm = KeyBindingManager::getInstance();
    std::string confirmKeyName = kbm->getKeyName(kbm->getKeyForAction(KeyBindingManager::Action::CONFIRM));
    // 动态生成提示文本
    std::string hintText = confirmKeyName + ": Start Game   ESC: Exit";

    _hintLabel2 = Label::createWithSystemFont(
        hintText,
        "Arial", 48
    );
    _hintLabel2->setColor(Color3B(160, 180, 160));
    _hintLabel2->setPosition(Vec2(visibleSize.width / 2, 230));
    this->addChild(_hintLabel2, 10);
}

void KeyBindingScene::updateKeyLabels()
{
    auto kbm = KeyBindingManager::getInstance();
    
    CCLOG("=== Updating Key Labels ===");
    
    // 添加安全检查
    if (_keyLabels.empty() || _actionOrder.empty())
    {
        CCLOG("ERROR: _keyLabels or _actionOrder is empty!");
        return;
    }
    
    for (size_t i = 0; i < _actionOrder.size(); i++)
    {
        auto action = _actionOrder[i];
        auto keyCode = kbm->getKeyForAction(action);
        std::string keyName = kbm->getKeyName(keyCode);
        
        CCLOG("  Action %zu: %s -> Key: %s", 
              i, 
              kbm->getActionName(action).c_str(),
              keyName.c_str());
        
        // Update the label text
        if (i < _keyLabels.size() && _keyLabels[i])
        {
            _keyLabels[i]->setString(keyName);
            CCLOG("    Label updated successfully");
        }
        else
        {
            CCLOG("    ERROR: Label is null or index out of range!");
        }
    }
    CCLOG("=========================");

    if (_hintLabel2)
    {
        std::string keyName = kbm->getKeyName(kbm->getKeyForAction(KeyBindingManager::Action::CONFIRM));

        if (_isPauseMode) {
            _hintLabel2->setString(keyName + ": Continue    ESC: Exit Game");
        }
        else {
            _hintLabel2->setString(keyName + ": Start Game");
        }
    }
}

void KeyBindingScene::updateSelector()
{
    // 添加安全检查
    if (_actionLabels.empty())
    {
        CCLOG("ERROR: _actionLabels is empty!");
        _selector->setVisible(false);
        return;
    }
    
    if (_currentSelection >= 0 && _currentSelection < (int)_actionLabels.size())
    {
        auto targetLabel = _actionLabels[_currentSelection];
        if (targetLabel)
        {
            auto visibleSize = Director::getInstance()->getVisibleSize();
            _selector->setPosition(Vec2(visibleSize.width / 2, targetLabel->getPositionY()));
            _selector->setVisible(true);
            
            // 添加脉动效果
            _selector->stopAllActions();
            auto fadeOut = FadeTo::create(0.6f, 80);
            auto fadeIn = FadeTo::create(0.6f, 150);
            auto sequence = Sequence::create(fadeOut, fadeIn, nullptr);
            _selector->runAction(RepeatForever::create(sequence));
        }
    }
    else
    {
        _selector->setVisible(false);
    }
}

void KeyBindingScene::setupKeyboardListener()
{
    auto listener = EventListenerKeyboard::create();
    
    listener->onKeyPressed = CC_CALLBACK_2(KeyBindingScene::onKeyPressed, this);
    listener->onKeyReleased = CC_CALLBACK_2(KeyBindingScene::onKeyReleased, this);
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void KeyBindingScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{   
    // 如果正在等待按键输入
    if (_isWaitingForKey)
    {
        CCLOG(">>> IN KEY LISTENING MODE <<<");
        
        // 排除不允许的按键
        if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE ||
            keyCode == EventKeyboard::KeyCode::KEY_ENTER)
        {
            CCLOG("  Key rejected (ESC or ENTER)");
            
            // 取消等待状态
            _isWaitingForKey = false;
            _keyLabels[_currentSelection]->stopAllActions();
            _keyLabels[_currentSelection]->setColor(Color3B(240, 240, 255));
            _keyLabels[_currentSelection]->setScale(1.0f);
            return;
        }
        
        CCLOG("  Accepting key for rebinding...");
        
        // 设置新按键
        auto kbm = KeyBindingManager::getInstance();
        kbm->setKeyForAction(_selectedAction, keyCode);
        
        CCLOG("  Key binding set in manager");
        
        // 更新显示
        updateKeyLabels();
        
        CCLOG("  Labels updated");
        
        // 恢复正常选择状态
        _isWaitingForKey = false;
        _keyLabels[_currentSelection]->stopAllActions();
        _keyLabels[_currentSelection]->setColor(Color3B(240, 240, 255));
        _keyLabels[_currentSelection]->setScale(1.0f);
                
        CCLOG("  Exited key listening mode");
        CCLOG("Key binding updated: %s -> %s",
              kbm->getActionName(_selectedAction).c_str(),
              kbm->getKeyName(keyCode).c_str());
        
        return;
    }
    
    CCLOG(">>> IN NAVIGATION MODE <<<");
    // 获取当前设置的“确认键”
    auto kbm = KeyBindingManager::getInstance();
    auto confirmKey = kbm->getKeyForAction(KeyBindingManager::Action::CONFIRM);
    auto pauseKey = kbm->getKeyForAction(KeyBindingManager::Action::PAUSE);
    // ============================================================
    // 暂停模式逻辑
    // ============================================================
    if (_isPauseMode)
    {
        // 如果按下了用户设置的“确认键” -> 继续游戏
        if (keyCode == confirmKey)
        {
            CCLOG("Action: Resume Game");
            resumeGame();
            return;
        }
        else if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
        {
            CCLOG("Action: Exit Game Application");
            Director::getInstance()->end();
            return;
        }
    }

    // ============================================================
    // 【修改】开始界面导航逻辑
    // ============================================================
    // 如果不是暂停模式，且按下了“确认键” -> 开始新游戏
    if (!_isPauseMode && keyCode == confirmKey)
    {
        CCLOG("Action: Start New Game");
        startGame(); // <--- 以前是 ESC 触发，现在改成自定义键触发
        return;
    }
    // 正常导航
    switch (keyCode)
    {
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
    case EventKeyboard::KeyCode::KEY_W:
        CCLOG("  Action: Move Up");
        moveSelectionUp();
        break;
        
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
    case EventKeyboard::KeyCode::KEY_S:
        CCLOG("  Action: Move Down");
        moveSelectionDown();
        break;
        
    case EventKeyboard::KeyCode::KEY_ENTER:
    case EventKeyboard::KeyCode::KEY_SPACE:
        CCLOG("  Action: Confirm Selection");
        confirmSelection();
        break;
        
    case EventKeyboard::KeyCode::KEY_ESCAPE:
        Director::getInstance()->end();
        return;
        break;
        
    case EventKeyboard::KeyCode::KEY_R:
        CCLOG("  Action: Reset Keys");
        resetKeys();
        break;
        
    default:
        CCLOG("  Unhandled key in navigation mode");
        break;
    }
    
    CCLOG("================================");
}

void KeyBindingScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
    // 暂不处理
}

void KeyBindingScene::moveSelectionUp()
{
    if (_isWaitingForKey) return;
    
    _currentSelection--;
    if (_currentSelection < 0)
    {
        _currentSelection = (int)_actionOrder.size() - 1;
    }
    updateSelector();
}

void KeyBindingScene::moveSelectionDown()
{
    if (_isWaitingForKey) return;
    
    _currentSelection++;
    if (_currentSelection >= (int)_actionOrder.size())
    {
        _currentSelection = 0;
    }
    updateSelector();
}

void KeyBindingScene::confirmSelection()
{
    CCLOG("========== CONFIRM SELECTION ==========");
    CCLOG("Current Selection: %d", _currentSelection);
    CCLOG("Is Waiting For Key (before): %s", _isWaitingForKey ? "YES" : "NO");
    
    if (_isWaitingForKey) {
        CCLOG("  Already waiting for key, ignoring");
        return;
    }
    
    // 添加安全检查
    if (_keyLabels.empty() || _actionOrder.empty())
    {
        CCLOG("ERROR: _keyLabels or _actionOrder is empty!");
        return;
    }
    
    if (_currentSelection < 0 || _currentSelection >= (int)_actionOrder.size())
    {
        CCLOG("ERROR: Invalid selection index!");
        return;
    }
    
    // 进入按键等待状态
    _isWaitingForKey = true;
    _selectedAction = _actionOrder[_currentSelection];
    
    CCLOG("  Selected Action: %s", 
          KeyBindingManager::getInstance()->getActionName(_selectedAction).c_str());
    
    // 高亮提示 - 使用更明显的金黄色
    if (_currentSelection < (int)_keyLabels.size() && _keyLabels[_currentSelection])
    {
        _keyLabels[_currentSelection]->setColor(Color3B(255, 240, 100));
        _keyLabels[_currentSelection]->setScale(1.1f);
        
        // 添加闪烁效果
        auto blink = Blink::create(2.0f, 4);
        _keyLabels[_currentSelection]->runAction(blink);
        
        CCLOG("  Label highlighted (yellow with blink)");
    }
    else
    {
        CCLOG("  ERROR: Label is null or index out of range!");
    }
    
    CCLOG("Is Waiting For Key (after): %s", _isWaitingForKey ? "YES" : "NO");
    CCLOG("=======================================");
    
    CCLOG("Waiting for key input for action: %s",
          KeyBindingManager::getInstance()->getActionName(_selectedAction).c_str());
}

void KeyBindingScene::startGame()
{
    CCLOG("Starting game...");
    
    // 保存配置
    KeyBindingManager::getInstance()->saveToFile();
    
    // 切换到游戏场景，使用淡入淡出效果
    auto scene = HelloWorld::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1.2f, scene, Color3B::BLACK));
}

void KeyBindingScene::resetKeys()
{
    if (_isWaitingForKey) return;
    
    // 添加安全检查
    if (_keyLabels.empty())
    {
        CCLOG("ERROR: _keyLabels is empty!");
        return;
    }
    
    KeyBindingManager::getInstance()->resetToDefault();
    updateKeyLabels();
    
    // 视觉反馈：闪烁所有按键标签
    for (auto label : _keyLabels)
    {
        if (label)  // 添加空指针检查
        {
            auto blink = Blink::create(0.5f, 2);
            label->runAction(blink);
        }
    }
    
    CCLOG("Key bindings reset to default");
}

void KeyBindingScene::setPauseMode(bool isPause)
{
    _isPauseMode = isPause;

    if (_isPauseMode)
    {
        // 1. 修改标题
        if (_titleLabel) {
            _titleLabel->setString("PAUSED");
            _titleLabel->setColor(Color3B::YELLOW); // 变成黄色提示暂停
        }

        // 2. 修改底部提示
        if (_hintLabel2) {
            auto kbm = KeyBindingManager::getInstance();
            std::string keyName = kbm->getKeyName(kbm->getKeyForAction(KeyBindingManager::Action::CONFIRM));
			_hintLabel2->setString(keyName + ": Continue    ESC: Exit Game");
        }
    }
}

void KeyBindingScene::resumeGame()
{
    // 保存配置
    KeyBindingManager::getInstance()->saveToFile();

    // 弹出当前场景，恢复到下一个场景 (HelloWorld)
    Director::getInstance()->popScene();
    CCLOG("Resuming Game...");
}