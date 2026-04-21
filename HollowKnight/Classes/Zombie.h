#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__

#include "cocos2d.h"
#include <functional> // 用 std::function
#include "GameEntity.h"

USING_NS_CC;

class Zombie : public GameEntity
{
public:
    // 僵尸状态枚举
    enum class State {
        PATROL,         // 巡逻
        ATTACK_READY,   // 准备攻击
        ATTACKING,      // 攻击中
        DAMAGED,        // 受伤
        DEAD            // 死亡
    };

    // 创建僵尸的静态方法
    static Zombie* create(const std::string& filename);

    // 初始化
    virtual bool init() override;

    // 【核心合并】每帧更新 (同时接收 玩家位置 和 平台数据)
    void update(float dt, const cocos2d::Vec2& playerPos, const std::vector<cocos2d::Rect>& platforms);

    // 受击处理
    void takeDamage(int damage, const cocos2d::Vec2& attackerPos) override;

    // 设置巡逻范围
    void setPatrolRange(float leftBound, float rightBound);

    // 获取碰撞箱
    cocos2d::Rect getHitbox() const override;

    // 碰到主角时的击退反应
    void onCollideWithPlayer(const cocos2d::Vec2& playerPos);

    // 定义回调类型
    typedef std::function<void()> DeathCallback;
    void setOnDeathCallback(DeathCallback callback) { _onDeathCallback = callback; }

    // 析构函数
    virtual ~Zombie();

private:
    // 状态管理
    State _currentState;
    void changeState(State newState);

    // 动画相关
    void loadAnimations();
    void playWalkAnimation();
    void playAttackReadyAnimation();
    void playAttackAnimation();
    void playDeathAnimation();

    // AI逻辑
    bool isPlayerInRange(const cocos2d::Vec2& playerPos);
    bool isPlayerInChaseRange(const cocos2d::Vec2& playerPos); // 【来自 File 2】追逐范围检测

    void updatePatrolBehavior(float dt);
    void updateAttackBehavior(float dt, const cocos2d::Vec2& playerPos);

    // 物理更新方法 (核心物理引擎)
    void updateMovementY(float dt);
    void updateCollisionY(const std::vector<cocos2d::Rect>& platforms);
    void updateMovementX(float dt);
    void updateCollisionX(const std::vector<cocos2d::Rect>& platforms);

    // 移动属性
    float _moveSpeed;           // 巡逻速度
    float _attackSpeed;         // 攻击移动速度
    bool _movingRight;          // 是否向右移动
    float _patrolLeftBound;     // 巡逻左边界
    float _patrolRightBound;    // 巡逻右边界

    // 检测范围
    float _detectionRange;      // 检测玩家的范围
    float _maxChaseRange;       
    cocos2d::Vec2 _spawnPosition; // 【来自 File 2】出生位置

    // 物理属性 (核心)
    cocos2d::Vec2 _velocity;    // 速度（包含X和Y方向）
    bool _isOnGround;           // 是否在地面上
    float _gravity;             // 重力加速度
    float _maxFallSpeed;        // 最大下落速度

    // 属性
    int _health;                // 生命值
    int _maxHealth;             // 最大生命值
    bool _isFacingRight;        // 面向方向

    // 受击无敌时间
    bool _isInvincible = false;

    // 动画缓存
    cocos2d::Animation* _walkAnimation = nullptr;
    cocos2d::Animation* _attackReadyAnimation = nullptr;
    cocos2d::Animation* _attackAnimation = nullptr;
    cocos2d::Animation* _deathAnimation = nullptr;

    // 回调函数
    DeathCallback _onDeathCallback = nullptr;
};

#endif // __ZOMBIE_H__