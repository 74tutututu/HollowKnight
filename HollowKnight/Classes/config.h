#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

#include "cocos2d.h"

namespace Config {
    struct PlayerStatConfig {
        int maxHealth = 5;       // 最大血量
        int maxSoul = 4;         // 最大魂量
        int healCost = 1;        // 回血消耗
        int soulGainPerKill = 1; // 杀一个怪，回 1 点 (即 1/4 瓶)
	};
    // 全局默认配置实例
    static const PlayerStatConfig DEFAULT_PLAYER_CFG;

    namespace Soul {
        // 动画参数
        const float FRAME_SPEED = 0.18f;    // 每帧播放速度 (0.1秒一帧)
        const float LOOP_DELAY = 3.0f;     // 动画播放完后等待 3 秒再播

        // 1. 灵魂瓶外框 (6帧动画)
        const std::string PATH_FRAME_ANIM = "HUDanim/HUDframe/HUDframe_%d.png";
        const std::string PATH_FRAME_STATIC = "HUDanim/HUDframe/HUDframe_6.png"; // 定格在第6帧
        const int FRAMES_FRAME_COUNT = 6;

        // 2. 液体 (现在都在 orb 文件夹下)
        const std::string PATH_EMPTY = "HUDanim/orb/V_empty/V_empty.png"; // 注意 orb
        const std::string PATH_QUARTER = "HUDanim/orb/V_quarter/V_quarter_%d.png";
        const std::string PATH_HALF = "HUDanim/orb/V_half/V_half_%d.png";
        const std::string PATH_3QUARTER = "HUDanim/orb/V_3quarter/V_3quarter_%d.png";
        const std::string PATH_FULL = "HUDanim/orb/V_full/V_full_%d.png";

        const int FRAMES_QUARTER = 4;
        const int FRAMES_HALF = 4;
        const int FRAMES_3QUARTER = 4;
        const int FRAMES_FULL = 5;
    }

        namespace Skill {
            const int FIREBALL_COST = 1;     // 消耗 1 点魂 (1/3 或 1/4 瓶)
            const float FIREBALL_SPEED = 600.0f; // 飞行速度
            const int FIREBALL_DAMAGE = 2;   // 伤害 (通常比平砍高)
        }

    namespace Health {
        // 血条路径
        const std::string PATH_FULL = "HUDanim/health/full/full.png";
        const std::string PATH_EMPTY = "HUDanim/health/empty/empty.png";

        // 出现动画 (5帧)
        const std::string PATH_APPEAR = "HUDanim/health/appear/appear_%d.png";
        const int FRAMES_APPEAR = 5;

        // 破碎动画 (6帧)
        const std::string PATH_BREAK = "HUDanim/health/break/break_%d.png";
        const int FRAMES_BREAK = 6;
    }

    namespace Player {
        // 物理参数
        const float MOVE_SPEED = 300.0f;
        const float GRAVITY = 2000.0f;
        const float MAX_FALL_SPEED = -1500.0f;

        // 手感参数
        const float MAX_JUMP_TIME = 0.35f;      // 长按跳跃最大时间
        const float JUMP_ACCEL = 3000.0f;       // 长按上升加速度
        const float JUMP_FORCE_BASE = 400.0f;

        // 战斗参数
        const float ATTACK_COOLDOWN = 0.04f;

        // 梦之钉配置
        const float DREAM_NAIL_CHARGE_TIME = 0.3f; // 蓄力需要多久 (秒)
        const float DREAM_NAIL_RANGE = 200.0f;     // 攻击距离 (比普通攻击远)
    }

    namespace Path {
        static const char* PLAYER_IDLE = "knight/idle/idle_%d.png";
        static const char* PLAYER_RUN = "knight/run/run_%d.png";
        static const char* PLAYER_JUMP = "knight/jump/jump_%d.png";
        static const char* PLAYER_FALL = "knight/fall/fall_%d.png";
        static const char* PLAYER_SLASH = "knight/slash/slash_%d.png";
        static const char* PLAYER_SLASH_EFFECT = "knight/slash/slashEffect/slashEffect_%d.png";
        static const char* PLAYER_DAMAGE = "knight/damage/damage_%d.png";
        static const char* PLAYER_LOOKUP = "knight/LookUp/LookUp_%d.png";
        static const char* PLAYER_LOOKDOWN = "knight/LookDown/LookDown_%d.png";
        static const char* PLAYER_UPSLASH = "knight/upslash/upslash_%d.png";
        static const char* PLAYER_DOWNSLASH = "knight/downslash/downslash_%d.png";
        static const char* PLAYER_UP_SLASH_EFFECT = "knight/upslash/upslasheffect/upslasheffect_%d.png";
        static const char* PLAYER_DOWN_SLASH_EFFECT = "knight/downslash/downslasheffect/downslasheffect_%d.png";
        static const char* PLAYER_DEATH = "knight/death/death_%d.png";
        static const char* PLAYER_FOCUS_LOOP = "knight/focus/focus_%d.png";
        static const char* PLAYER_FOCUS_GET = "knight/focusGet/focusGet_%d.png";
        static const char* PLAYER_FOCUS_END = "knight/focusEnd/focusEnd_%d.png";
        static const char* PLAYER_DAMAGED = "knight/recoil/recoil_%d.png";
        static const char* PLAYER_CAST_ANTIC = "knight/fireballAntic/fireballAntic_%d.png";
        static const char* PLAYER_CAST_RELEASE = "knight/fireballCast/fireballCast_%d.png";        static const char* PLAYER_ANTIC = "knight/fireballAntic/fireballAntic_%d.png";
        static const char* EFFECT_FOCUS_LOOP = "knight/focusGet/focusEffect/focusEffect_%d.png";
        static const char* EFFECT_FOCUS_END = "knight/focusEnd/focusEffectEnd/focusEffectEnd_%d.png";
        static const char* DREAM_NAIL_CHARGE = "Knight/DNcharge/DNcharge_%d.png";
        static const char* DREAM_NAIL_SLASH = "Knight/DNslash/DNslash_%d.png";

        // 火球飞行特效
        static const std::string FIREBALL_IDLE = "fireball/idle/fireball_%d.png";
        static const std::string FIREBALL_FLY = "fireball/fly/fly_%d.png";
   
		// 梦之钉对话框
        static const std::string DREAM_DIALOGUE_UP = "dialogue/dreamUp/dreamUp_%d.png";
        static const std::string DREAM_DIALOGUE_DOWN = "dialogue/dreamDown/dreamDown_%d.png";
    }

    // 音频路径配置 
    namespace Audio {
        // 背景音乐
        static const char* BGM_DIRTMOUTH = "audio/Royal_HollowKnight_Theme.mp3"; 
        static const char* BGM_BOSS = "audio/BossBattle.mp3";
        // 主角动作
        static const char* HERO_JUMP = "audio/hero_jump.mp3";
        static const char* HERO_LAND_SOFT = "audio/hero_land_soft.mp3";
        static const char* HERO_LAND_HARD = "audio/hero_land_hard.mp3";
        static const char* HERO_RUN = "audio/hero_run_footsteps_stone.mp3";
        static const char* HERO_CAST = "audio/hero_fireball.mp3";
        // 战斗
        static const char* HERO_DAMAGE = "audio/hero_damage.mp3";
        static const char* HERO_DEATH = "audio/hero_death_v2.mp3";
        static const char* HERO_UNSHEATH = "audio/hero_unsheath.mp3";

        // 攻击 (随机池)
        static const char* SWORD_1 = "audio/sword_1.mp3";
        static const char* SWORD_2 = "audio/sword_2.mp3";
        static const char* SWORD_3 = "audio/sword_3.mp3";

        // 技能 (凝聚)
        static const char* FOCUS_CHARGE = "audio/focus_health_charging.mp3";
        static const char* FOCUS_HEAL = "audio/focus_health_heal.mp3";
    }

    namespace Render {
        const int Z_ORDER_PLAYER = 10;
        const int Z_ORDER_ENEMY = 5;
        const int Z_ORDER_MAP = -99;
    }
}

#endif