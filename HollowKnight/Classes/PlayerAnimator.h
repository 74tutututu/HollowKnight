#ifndef __PLAYER_ANIMATOR_H__
#define __PLAYER_ANIMATOR_H__

#include "cocos2d.h"
#include <string>

// 前向声明，避免循环引用
class Player;

class PlayerAnimator
{
public:
    PlayerAnimator();
    ~PlayerAnimator();

    // 初始化：传入 owner (主角)，以便把特效精灵加到主角身上
    void init(cocos2d::Sprite* owner);
    static void preloadSounds();

    // --- 核心接口 ---
    // 播放主角本体动画
    void playAnimation(const std::string& animName);

    // --- 特效接口 ---
    // 播放攻击刀光 (根据方向)
    void playAttackEffect(int dir, bool facingRight);

    // 凝聚特效控制
    void startFocusEffect();
    void stopFocusEffect();
    void playFocusEndEffect();

private:
    // 内部加载函数
    void loadAllAnimations();
    void loadAnim(const std::string& name, const std::string& format, int count, float delay);


private:
    // 持有主角的引用 (用于 runAction)
    cocos2d::Sprite* _owner;

    // 动画缓存
    cocos2d::Map<std::string, cocos2d::Animation*> _animations;

    // --- 特效精灵 (现在归 Animator 管) ---
    cocos2d::Sprite* _slashEffectSprite; // 刀光
    cocos2d::Sprite* _focusEffectSprite; // 凝聚光效
};

#endif