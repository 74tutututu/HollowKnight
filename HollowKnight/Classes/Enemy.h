#ifndef __ENEMY_H__
#define __ENEMY_H__
#include <functional> 
#include "cocos2d.h"
#include "GameEntity.h" // 引入基类

USING_NS_CC;

class Enemy : public GameEntity
{
public:
    // 敌人状态枚举
    enum class State {
        PATROL,     // 巡逻
        DEAD        // 死亡
    };

    // 创建敌人的静态方法
    static Enemy* create(const std::string& filename);

    // 初始化
    virtual bool init() override;

    // 每帧更新
    void update(float dt) override;

    // 设置巡逻范围
    void setPatrolRange(float leftBound, float rightBound);

    // 获取碰撞箱（用于攻击判定）
    virtual cocos2d::Rect getHitbox() const override;

    // 碰到主角时的击退反应
    virtual void takeDamage(int damage, const cocos2d::Vec2& sourcePos) override;

    void onCollideWithPlayer(const cocos2d::Vec2& playerPos);

    // 定义回调类型：这是一个“没有参数，也没有返回值”的函数类型
    typedef std::function<void()> DeathCallback;

    // 设置回调的函数
    void setOnDeathCallback(DeathCallback callback) { _onDeathCallback = callback; }

    // 析构函数
    virtual ~Enemy();

private:
    // 状态管理
    State _currentState;
    void changeState(State newState);

    // 动画相关
    void loadAnimations();
    void playWalkAnimation();
    void playDeathAnimation();

    // 移动相关
    float _moveSpeed;           // 移动速度
    bool _movingRight;          // 是否向右移动
    float _patrolLeftBound;     // 巡逻左边界
    float _patrolRightBound;    // 巡逻右边界

    // 属性
    int _health;                // 生命值
    int _maxHealth;             // 最大生命值

    // 存储回调函数
    DeathCallback _onDeathCallback = nullptr;

    // 受击无敌时间
    bool _isInvincible = false;

    // 动画缓存
    cocos2d::Animation* _walkAnimation;
    cocos2d::Animation* _deathAnimation;
};

#endif // __ENEMY_H__#pragma once
