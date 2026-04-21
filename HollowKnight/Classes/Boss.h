#ifndef __BOSS_H__
#define __BOSS_H__

#include "cocos2d.h"
#include "FKFireball.h"
#include "GameEntity.h"

class Boss : public cocos2d::Node
{
public:
    enum class State {
        None,
        Falling_Enter, // 入场下落
        Idle,          // 待机
        Pre_Jump,      // 起跳前摇
        Jumping,       // 普通跳跃
        Pre_Attack,    // 攻击前摇
        Jump_Attack,   // 跳跃攻击
        Shockwave_Attack, // 发波攻击
        Stunned,       // 瘫痪（包括播放动画和静止）
        Recovering,     // 起身恢复（本项目中作为“从瘫痪恢复”的流程入口）
        Rampage_Jump,   // 狂暴突进跳
        Rampage_Attack  // 狂暴攻击
    };

    static Boss* create(const cocos2d::Vec2& spawnPos);
    virtual bool init(const cocos2d::Vec2& spawnPos);

    // 核心更新循环
    void updateBoss(float dt, const cocos2d::Vec2& playerPos, const std::vector<cocos2d::Rect>& groundRects);

    // 伤害接口
    // damage: 扣除的数值 (平A=1, 法术=2)
     void takeDamage(int damage) ;

    // 获取碰撞框
    cocos2d::Rect getBodyHitbox() const ;
    cocos2d::Rect getHammerHitbox() const;

    bool isDead() const { return _isDead; }

    // 设置生成火球的回调
    void setFireballCallback(const std::function<void(const cocos2d::Vec2&)>& callback) {
        _fireballCallback = callback;
    }
    void setShockwaveCallback(const std::function<void(const cocos2d::Vec2&, float)>& callback) {
        _shockwaveCallback = callback;
    }

private:
    // 动画辅助
    void playAnimation(std::string name, int startFrame, int frameCount, bool loop, float delay = 0.1f, std::function<void()> onComplete = nullptr);
    void setFacing(float playerX);
    void applyFacing(float newFacing); // 根据目标朝向调整自身位置与翻转
    float getForwardOffset() const;    // 计算身体碰撞框前移偏移量
    void scheduleHammerWindow(float startDelay, float duration); // 控制锤子判定窗口

    // 状态机逻辑
    void switchState(State newState);
    void onLand(); // 落地回调

    // 行为逻辑
    void performNextAction(); // 决定下一步干什么
    void recoverFromStun();   // 从瘫痪中恢复（达到阈值/超时）
    void startRampage();      // 开始狂暴模式
    void rampageAttack();     // 执行狂暴攻击
    void rampageAttackLoop(int count); // 狂暴攻击循环

    // 死亡处理
    void die();

    cocos2d::Sprite* _sprite;
    cocos2d::Vec2 _velocity;
    State _state;

    // 属性
    int _hitCount;          // 非瘫痪期：累计受击“硬直值”（满13进入瘫痪）
    int _stunHitCount;      // 单次瘫痪内：累计受到的“伤害点数”（平A=1, 法术=2），满6立刻起身
    int _stunHP;            // Boss“瘫痪期血量”：只在 Stunned 状态下扣血，总计18点，清空立刻死亡

    float _stunTimer;       // 瘫痪计时器 (10s)
    bool _isStunAnimPlaying;// 标记瘫痪动画是否正在播放

    float _hurtTimer;       // 受击无敌时间
    float _lastPlayerX;     // 记录主角X位置，用于计算跳跃距离
    float _facing;          // 当前朝向：1 右 -1 左

    bool _isDead;
    bool _onGround;         // 是否在地面

    // 标记攻击跳跃是否已经处理过落地逻辑
    bool _isAttackLanded;

    // 标记锤子是否有伤害 (只在判定窗口开启)
    bool _isHammerActive;

    // 狂暴攻击相关
    int _rampageCounter;         // 狂暴攻击循环计数器
    float _rampageFireballTimer; // 狂暴火球生成计时器
    bool _isRampaging;           // 是否处于狂暴火球阶段

    // 动作序列控制
    int _actionStep;        // 0:Idle, 1:Jump, 2:Idle, 3:Attack
    float _stateTimer;      // 通用计时器
    int _pendingIdleCount;  // 每个招式后必须播放的Idle轮数

    // 闪白效果
    void flashEffect();

    // 回调
    std::function<void(const cocos2d::Vec2&)> _fireballCallback;
    std::function<void(const cocos2d::Vec2&, float)> _shockwaveCallback;
};

#endif // __BOSS_H__