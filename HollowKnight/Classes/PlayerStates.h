#ifndef __PLAYER_STATES_H__
#define __PLAYER_STATES_H__

#include "PlayerState.h" // 包含基类

// =========================
// 待机
// =========================
class StateIdle : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
};

// =========================
// 跑步
// =========================
class StateRun : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
};

// =========================
// 跳跃 (上升)
// =========================
class StateJump : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
};

// =========================
// 下落
// =========================
class StateFall : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
};

// =========================
// 攻击
// =========================
class StateSlash : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
private:
    float _timer;
    float _duration;
};

// =========================
// 受伤
// =========================
class StateDamaged : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
private:
    float _timer;
    float _duration;
};

// =========================
// 仰望
// =========================
class StateLookUp : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
private:
    float _timer;
    float _duration;
};

// =========================
// 俯视
// =========================
class StateLookDown : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
private:
    float _timer;
    float _duration;
};

// 上劈状态
class StateSlashUp : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
private:
    float _timer; float _duration;
};

// 下劈状态
class StateSlashDown : public PlayerState {
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;
private:
    float _timer; float _duration;
};

// ==========================================
// 【新增】凝聚状态 
// ==========================================
class StateFocus : public PlayerState
{
public:
    virtual void enter(Player* player) override;
    virtual void update(Player* player, float dt) override;
    virtual void exit(Player* player) override;

private:
    float _timer;
    bool _hasHealed;      // 标记是否已回血
    bool _isEnding;       // 标记是否进入了收尾(End)阶段
};

// ==========================================
// 死亡状态
// ==========================================
class StateDead : public PlayerState
{
public:
    virtual void enter(Player* player) override;
    virtual void update(Player* player, float dt) override;
    virtual void exit(Player* player) override;
};
#endif // __PLAYER_STATES_H__

class StateCast : public PlayerState
{
public:
    virtual void enter(Player* player);
    virtual void update(Player* player, float dt) override;
    virtual void exit(Player* player) override;

private:
    float _timer;
    bool _hasSpawned; // 标记是否已经生成了火球
};

// 梦之钉状态
class StateDreamNail : public PlayerState
{
public:
    void enter(Player* player) override;
    void update(Player* player, float dt) override;
    void exit(Player* player) override;

private:
    float _timer;
    bool _hasSlashed;
};