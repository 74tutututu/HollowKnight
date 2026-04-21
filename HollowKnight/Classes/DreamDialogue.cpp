#include "DreamDialogue.h"
#include "Config.h" 

USING_NS_CC;

DreamDialogue* DreamDialogue::create(const std::string& text) {
    DreamDialogue* pRet = new(std::nothrow) DreamDialogue();
    if (pRet && pRet->init(text)) {
        pRet->autorelease();
        return pRet;
    }
    delete pRet;
    return nullptr;
}

bool DreamDialogue::init(const std::string& text) {
    if (!Node::init()) return false;

    // 1. 创建背景 (默认第一帧)
    std::string path = StringUtils::format(Config::Path::DREAM_DIALOGUE_UP.c_str(), 1);
    
    _bg = Sprite::create(path);
    if (_bg) {
        this->addChild(_bg);
    }

    // 2. 创建文字
    _label = Label::createWithTTF(text, "fonts/Marker Felt.ttf", 60);
    if (_label && _bg) {
        _label->setPosition(_bg->getContentSize().width/2, _bg->getContentSize().height/2+30.0f); // 居中
        _label->setTextColor(Color4B::WHITE);
        _label->enableOutline(Color4B::BLACK, 1);
        _label->setDimensions(_bg->getContentSize().width * 0.8f, 0); // 自动换行
        _label->setAlignment(TextHAlignment::CENTER);
        _label->setOpacity(0); // 初始隐藏
        _bg->addChild(_label);
    }

    return true;
}

void DreamDialogue::show()
{
    if (!_bg) return;

    // =============================================================
    // 1. 加载【出现】动画 (dreamUp 1~5)
    // =============================================================
    Vector<SpriteFrame*> upFrames;
    for (int i = 1; i <= 5; i++)
    {
        // 拼接路径：dialogue/dreamUp/dreamUp_1.png ...
        std::string path = StringUtils::format(Config::Path::DREAM_DIALOGUE_UP.c_str(), i);

        auto texture = Director::getInstance()->getTextureCache()->addImage(path);
        if (texture) {
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
            upFrames.pushBack(frame);
        }
        else {
            CCLOG("Error: Frame not found: %s", path.c_str());
        }
    }
    // 创建动画对象：0.05秒一帧，很快弹出
    auto animUp = Animation::createWithSpriteFrames(upFrames, 0.05f);

    // =============================================================
    // 2. 加载【消失】动画 (dreamDown 1~5)
    // =============================================================
    Vector<SpriteFrame*> downFrames;
    for (int i = 1; i <= 5; i++)
    {
        // 拼接路径：dialogue/dreamDown/dreamDown_1.png ...
        std::string path = StringUtils::format(Config::Path::DREAM_DIALOGUE_DOWN.c_str(), i);

        auto texture = Director::getInstance()->getTextureCache()->addImage(path);
        if (texture) {
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
            downFrames.pushBack(frame);
        }
    }
    // 也是 0.05秒一帧
    auto animDown = Animation::createWithSpriteFrames(downFrames, 0.05f);

    // =============================================================
    // 3. 组合动作序列 (Sequence)
    // =============================================================

    // 动作A: 文字淡入
    auto showTextAction = CallFunc::create([this]() {
        if (_label) _label->runAction(FadeIn::create(0.2f));
        });

    // 动作B: 文字淡出
    auto hideTextAction = CallFunc::create([this]() {
        if (_label) _label->runAction(FadeOut::create(0.2f));
        });

    // 动作C: 自我销毁 (从父节点移除)
    auto killSelf = RemoveSelf::create();

    // 编排剧本：
    auto seq = Sequence::create(
        Animate::create(animUp),      // 1. 气泡从小变大弹出
        showTextAction,               // 2. 文字显现
        DelayTime::create(5.0f),      // 3. 停留阅读5秒
        hideTextAction,               // 4. 文字先消失
        DelayTime::create(0.2f),      // 5. 稍微等一下
        Animate::create(animDown),    // 6. 气泡从大变小消失
        killSelf,                     // 7. 删除这个对象，释放内存
        nullptr
    );

    // 开始执行
    _bg->runAction(seq);
}