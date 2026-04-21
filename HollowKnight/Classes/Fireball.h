#ifndef __FIREBALL_H__
#define __FIREBALL_H__

#include "cocos2d.h"

USING_NS_CC;

class Fireball : public Sprite
{
public:
    static Fireball* create(const std::string& firstFrame);

    virtual bool init(const std::string& firstFrame);

    // 播放循环动画
    void playAnimation();

    // 发射火球
    // direction: 1 (右), -1 (左)
    void shoot(float speed, int direction);

    void update(float dt) override;

    // 获取碰撞箱 (比图片稍小一点，避免空气伤害)
    Rect getHitbox() const;

    // 标记是否已经造成过伤害 (防止对同一个怪每帧都扣血)
    bool hasHitEnemy(int enemyTag);
    void addHitEnemy(int enemyTag);

    // 播放待机动画 (作为物品时)
    void playIdleAnimation();

    // 播放飞行动画 (作为子弹时)
    void playFlyAnimation();
private:
    // 动画相关
    void loadAnimation();

    Vec2 _velocity;       // 速度向量
    bool _isLaunched;     // 是否发射出去了
    float _lifeTime;      // 存在时间 (防止飞出地图无限存在)
    std::vector<int> _hitEnemyTags; // 记录打到过的敌人Tag
};

#endif // __FIREBALL_H__
