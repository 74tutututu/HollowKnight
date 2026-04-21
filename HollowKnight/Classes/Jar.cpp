#include "Jar.h"
#include "DreamDialogue.h"

Jar* Jar::create(const std::string& jarImage, const Vec2& position)
{
    Jar* jar = new (std::nothrow) Jar();
    if (jar && jar->init(jarImage, position))
    {
        jar->autorelease();
        return jar;
    }
    CC_SAFE_DELETE(jar);
    return nullptr;
}

bool Jar::init(const std::string& jarImage, const Vec2& position)
{
    if (!Sprite::init())
    {
        return false;
    }

    _health = 5;
    _isDestroyed = false;
    _isInvincible = false;

    // 1. 创建罐子精灵
    _jarSprite = Sprite::create(jarImage);
    if (!_jarSprite)
    {
        CCLOG("Error: Failed to load jar image: %s", jarImage.c_str());
        return false;
    }
    
    _jarSprite->setAnchorPoint(Vec2(0.5f, 0.0f)); // 底部对齐
    this->addChild(_jarSprite, 1);
    
    // 2. 创建幼虫精灵（在罐子上方）
    _grubSprite = Sprite::create("warm/attach/attach_1.png");
    if (!_grubSprite)
    {
        CCLOG("Error: Failed to load grub image");
        return false;
    }
    
    _grubSprite->setAnchorPoint(Vec2(0.5f, 0.0f));
    
    // 幼虫位置：罐子内部，向下调整
    float jarHeight = _jarSprite->getContentSize().height;
    _grubSprite->setPosition(Vec2(0, jarHeight * 0.2f)); // 【修改】降低高度，让幼虫看起来在罐子内部
    
    this->addChild(_grubSprite, 2); // 幼虫在罐子上层
    
    // 3. 设置节点位置
    this->setPosition(position);
    
    // 4. 播放幼虫附着动画
    playGrubAttachAnimation();
    
    CCLOG("Jar created at position (%.0f, %.0f)", position.x, position.y);
    
    return true;
}

// =========================================================
//  创建三个不同的罐子，用于谜题场景
// =========================================================
void Jar::setupPuzzleJars(Node* parent, Vector<Jar*>& outList)
{
    if (!parent) return;

    // 基础配置：根据你 loadMap 里的原有数据
    const std::string imagePath = "warm/jar.png"; // 修正路径
    const float jarY = 346.0f;                    // 修正高度
    const int zOrder = 5;                         // 修正层级

    // -----------------------------------------------------
    // 罐子 1：位置 1700 (错误的)
    // -----------------------------------------------------
    auto jar1 = Jar::create(imagePath, Vec2(1700.0f, jarY));
    if (jar1) {
        jar1->setDreamThought("Save me...I hold the flame..."); // 梦之钉心声
        jar1->setTag(990); // 保持原有的 tag 逻辑，或者不设也行

        parent->addChild(jar1, zOrder);
        outList.pushBack(jar1);
    }

    // -----------------------------------------------------
    // 罐子 2：位置 2400 (空的)
    // -----------------------------------------------------
    auto jar2 = Jar::create(imagePath, Vec2(2400.0f, jarY));
    if (jar2) {
        jar2->setDreamThought("...The first one... holds the flame....");
        jar2->setTag(989);

        parent->addChild(jar2, zOrder);
        outList.pushBack(jar2);
    }

    // -----------------------------------------------------
    // 罐子 3：位置 3100 
    // -----------------------------------------------------
    auto jar3 = Jar::create(imagePath, Vec2(3100.0f, jarY));
    if (jar3) {
        jar3->setDreamThought("...The middle one... is empty..."); // 提示玩家

        // 【关键】打上特殊的标签 888，用于 HelloWorld 检测并给技能
        jar3->setTag(888);

        parent->addChild(jar3, zOrder);
        outList.pushBack(jar3);
    }

    CCLOG("Puzzle Jars created for Level 2 at correct positions!");
}

Rect Jar::getHitbox() const
{
    return this->getCollisionBox(); 
}

Rect Jar::getCollisionBox() const
{
    if (_isDestroyed || !_jarSprite)
    {
        return Rect::ZERO;
    }
    
    // 罐子的碰撞箱（侧面，防止穿过）
    Size jarSize = _jarSprite->getContentSize();
    Vec2 worldPos = this->getPosition();
    
    // 碰撞箱稍微缩小一点，更精确
    float boxWidth = jarSize.width * 0.8f;
    float boxHeight = jarSize.height * 0.9f;
    
    return Rect(
        worldPos.x - boxWidth / 2,
        worldPos.y,
        boxWidth,
        boxHeight
    );
}

// 多态的核心：罐子自己决定被梦钉砍了怎么做
void Jar::onDreamNailHit()
{
    if (this->isDestroyed()) return;

    //  飘字
    std::string text = this->getDreamThought();
    if (!text.empty()) {
        auto dialogue = DreamDialogue::create(text);
        if (dialogue) {
            dialogue->setPosition(this->getPosition() + Vec2(0, 60));
            this->getParent()->addChild(dialogue, 200);
            dialogue->show();
        }
    }

    CCLOG("Jar Dream Logic Executed!");
}

Rect Jar::getTopPlatformBox() const
{
    if (_isDestroyed || !_jarSprite)
    {
        return Rect::ZERO;
    }
    
    // 罐子顶部平台（用于站立）
    Size jarSize = _jarSprite->getContentSize();
    Vec2 worldPos = this->getPosition();
    
    // 顶部平台：比罐子宽度略窄，高度很薄
    float platformWidth = jarSize.width * 0.9f;  // 顶部宽度
    float platformHeight = 20.0f;  // 平台厚度
    float topY = worldPos.y + jarSize.height * 0.85f;  // 罐子顶部位置
    
    return Rect(
        worldPos.x - platformWidth / 2,
        topY,
        platformWidth,
        platformHeight
    );
}

void Jar::takeDamage()
{
    if (_isDestroyed || _isInvincible) return;
    _isInvincible = true;
    _health--;

    this->scheduleOnce([this](float dt) {
        _isInvincible = false;
        }, 0.5f, "jar_invincible_key");

    if (_health > 0)
    {
        CCLOG("Jar hit! HP remaining: %d", _health);

        // 左右抖动效果
        auto moveLeft = MoveBy::create(0.05f, Vec2(-5, 0));
        auto moveRight = MoveBy::create(0.05f, Vec2(10, 0));
        auto moveBack = MoveBy::create(0.05f, Vec2(-5, 0));
        auto shake = Sequence::create(moveLeft, moveRight, moveBack, nullptr);

        // 简单的变色反馈
        if (_jarSprite) {
            _jarSprite->runAction(Sequence::create(
                TintTo::create(0.1f, 200, 200, 200), // 变暗一点
                TintTo::create(0.1f, 255, 255, 255), // 恢复
                nullptr
            ));
        }

        this->runAction(shake);
        return; // 【关键】直接返回，不执行下面的破碎逻辑
    }

    _isDestroyed = true;
    
    // 罐子消失
    if (_jarSprite)
    {
        _jarSprite->runAction(Sequence::create(
            FadeOut::create(0.2f),
            RemoveSelf::create(),
            nullptr
        ));
    }
    
    // 播放幼虫释放动画
    playGrubFreeAnimation();

    // ==========================================
    // 触发回调
    // ==========================================
    if (_onBrokenCallback)
    {
        _onBrokenCallback();
    }
}

void Jar::playGrubAttachAnimation()
{
    if (!_grubSprite) return;
    
    // 创建 attach 动画（5帧，循环播放）
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= 5; i++)
    {
        std::string frameName = StringUtils::format("warm/attach/attach_%d.png", i);
        auto texture = Director::getInstance()->getTextureCache()->addImage(frameName);
        if (texture)
        {
            Size texSize = texture->getContentSize();
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texSize.width, texSize.height));
            if (frame)
            {
                frames.pushBack(frame);
            }
        }
    }
    
    if (frames.size() > 0)
    {
        auto animation = Animation::createWithSpriteFrames(frames, 0.15f); // 每帧0.15秒
        auto animate = Animate::create(animation);
        auto repeatForever = RepeatForever::create(animate);
        
        _grubSprite->runAction(repeatForever);
        CCLOG("Grub attach animation started (looping)");
    }
    else
    {
        CCLOG("Warning: Failed to load attach animation frames");
    }
}

void Jar::playGrubFreeAnimation()
{
    if (!_grubSprite) return;
    
    // 停止之前的动画
    _grubSprite->stopAllActions();
    
    // 创建 free 动画（6帧，播放一次）
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= 6; i++)
    {
        std::string frameName = StringUtils::format("warm/free/free_%d.png", i);
        auto texture = Director::getInstance()->getTextureCache()->addImage(frameName);
        if (texture)
        {
            Size texSize = texture->getContentSize();
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texSize.width, texSize.height));
            if (frame)
            {
                frames.pushBack(frame);
            }
        }
    }
    
    if (frames.size() > 0)
    {
        auto animation = Animation::createWithSpriteFrames(frames, 0.1f); // 每帧0.1秒
        auto animate = Animate::create(animation);
        
        // 播放完后淡出并移除
        auto fadeOut = FadeOut::create(0.3f);
        auto remove = RemoveSelf::create();
        
        _grubSprite->runAction(Sequence::create(
            animate,
            fadeOut,
            remove,
            nullptr
        ));
        
        CCLOG("Grub free animation started (once)");
        
        // 延迟后移除整个节点
        this->runAction(Sequence::create(
            DelayTime::create(1.5f),
            RemoveSelf::create(),
            nullptr
        ));
    }
    else
    {
        CCLOG("Warning: Failed to load free animation frames");
    }
}

void Jar::onJarBroken()
{
    // 可以在这里添加破碎音效、粒子特效等
}
