#ifndef __BUZZER_H__
#define __BUZZER_H__
#include "GameEntity.h"
#include "cocos2d.h"
#include <functional>

class Buzzer : public GameEntity
{
public:
    // 状态枚举
    enum class State
    {
        IDLE,       // 悬停状态
        ATTACKING,  // 攻击/追击状态
        DEAD        // 死亡状态
    };

    // 创建方法
    static Buzzer* create(const std::string& filename);
    virtual bool init() override;

    // 更新方法（AI逻辑）
    void update(float dt, const cocos2d::Vec2& playerPos);

    // 受伤方法
    void takeDamage(int damage, const cocos2d::Vec2& attackerPos) override;

    // 与玩家碰撞时的回调
    void onCollideWithPlayer(const cocos2d::Vec2& playerPos);

    // 获取碰撞箱
    cocos2d::Rect getHitbox() const override;

    // 设置初始位置
    void setInitialPosition(const cocos2d::Vec2& pos);

    // 析构函数
    ~Buzzer();

    // ==========================================
    // 回魂机制接口
    // ==========================================
    typedef std::function<void()> DeathCallback;
    void setOnDeathCallback(DeathCallback callback) { _onDeathCallback = callback; }

private:
    // 状态相关
    State _currentState;
    void changeState(State newState);

    // 动画相关
    cocos2d::Animation* _idleAnimation;
    cocos2d::Animation* _attackAnimation;
    void loadAnimations();
    void playIdleAnimation();
    void playAttackAnimation();
    void playDeathAnimation();

    // 属性
    int _health;
    int _maxHealth;
    bool _isInvincible;

    // AI相关
    float _detectionRange;  // 检测范围
    float _chaseSpeed;      // 追击速度
    cocos2d::Vec2 _initialPosition;  // 初始位置
    cocos2d::Vec2 _velocity;         // 当前速度

    // 辅助方法
    bool isPlayerInRange(const cocos2d::Vec2& playerPos);
    void chasePlayer(const cocos2d::Vec2& playerPos, float dt);

    // 【新增】保存回调函数
    DeathCallback _onDeathCallback = nullptr;
};

#endif // __BUZZER_H__
