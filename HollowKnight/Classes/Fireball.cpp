#include "Fireball.h"
#include "config.h"

Fireball* Fireball::create(const std::string& firstFrame)
{
    Fireball* fireball = new (std::nothrow) Fireball();
    if (fireball && fireball->init(firstFrame))
    {
        fireball->autorelease();
        return fireball;
    }
    CC_SAFE_DELETE(fireball);
    return nullptr;
}

bool Fireball::init(const std::string& firstFrame)
{
    if (!Sprite::init())
    {
        return false;
    }
    
    // 加载第一帧图片
    auto texture = Director::getInstance()->getTextureCache()->addImage(firstFrame);
    if (texture)
    {
        this->setTexture(texture);
        Size texSize = texture->getContentSize();
        this->setTextureRect(Rect(0, 0, texSize.width, texSize.height));
    }
    else
    {
        CCLOG("Error: Failed to load fireball first frame: %s", firstFrame.c_str());
        return false;
    }
    
    // 设置锚点为中心
    this->setAnchorPoint(Vec2(0.5f, 0.5f));
    
    // 播放动画
    playIdleAnimation();
    //CCLOG("Fireball created successfully");
    
    // 初始化变量
    _velocity = Vec2::ZERO;
    _isLaunched = false;
    _lifeTime = 0.0f;

    // 开启 Update
    this->scheduleUpdate();

    return true;
}

void Fireball::playIdleAnimation()
{
    this->stopAllActions(); // 停止之前的动画

    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= 4; i++)
    {
        // 使用 Config 定义的 IDLE 路径
        std::string path = StringUtils::format(Config::Path::FIREBALL_IDLE.c_str(), i);
        auto texture = Director::getInstance()->getTextureCache()->addImage(path);
        if (texture) {
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
            frames.pushBack(frame);
        }
    }

    if (frames.size() > 0)
    {
        // 待机动画通常比较慢，0.15秒一帧
        auto animation = Animation::createWithSpriteFrames(frames, 0.15f);
        this->runAction(RepeatForever::create(Animate::create(animation)));
    }
}

// 2. 播放飞行动画 (Projectile)
void Fireball::playFlyAnimation()
{
    this->stopAllActions(); // 停止待机动画

    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= 4; i++)
    {
        // 使用 Config 定义的 FLY 路径
        std::string path = StringUtils::format(Config::Path::FIREBALL_FLY.c_str(), i);
        auto texture = Director::getInstance()->getTextureCache()->addImage(path);
        if (texture) {
            auto frame = SpriteFrame::createWithTexture(texture, Rect(0, 0, texture->getContentSize().width, texture->getContentSize().height));
            frames.pushBack(frame);
        }
    }

    if (frames.size() > 0)
    {
        // 飞行极快，0.05秒一帧
        auto animation = Animation::createWithSpriteFrames(frames, 0.05f);
        this->runAction(RepeatForever::create(Animate::create(animation)));
    }
}

// 3. 发射逻辑 (自动切换动画)
void Fireball::shoot(float speed, int direction)
{
    // 一旦发射，立刻切换到飞行表现
    playFlyAnimation();

    _velocity = Vec2(speed * direction, 0);
    _isLaunched = true;

    // 调整方向和锚点
    if (direction < 0) {
        this->setFlippedX(true);
        this->setAnchorPoint(Vec2(0.6f, 0.5f));
    }
    else {
        this->setFlippedX(false);
        this->setAnchorPoint(Vec2(0.4f, 0.5f));
    }

}

// 【新增】核心更新
void Fireball::update(float dt)
{
    // 1. 如果还没发射 (只是地图上的道具)，不需要移动
    if (!_isLaunched) return;

    // 2. 移动
    this->setPosition(this->getPosition() + _velocity * dt);

    // 3. 生命周期管理 (2秒后自动销毁，或者飞出屏幕太远)
    _lifeTime += dt;
    if (_lifeTime > 2.0f) {
        this->removeFromParent();
        return;
    }
}

// 【新增】获取碰撞箱
Rect Fireball::getHitbox() const
{
    // 获取稍微缩小一点的碰撞箱，手感更好
    Rect rect = this->getBoundingBox();
    rect.origin.x += rect.size.width * 0.2f;
    rect.origin.y += rect.size.height * 0.2f;
    rect.size.width *= 0.6f;
    rect.size.height *= 0.6f;
    return rect;
}

bool Fireball::hasHitEnemy(int enemyTag)
{
    for (int tag : _hitEnemyTags) {
        if (tag == enemyTag) return true;
    }
    return false;
}

void Fireball::addHitEnemy(int enemyTag)
{
    _hitEnemyTags.push_back(enemyTag);
}

void Fireball::loadAnimation()
{
    // 备用方法，如果需要的话
}
