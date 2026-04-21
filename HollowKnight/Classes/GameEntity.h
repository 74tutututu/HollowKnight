#ifndef __GAME_ENTITY_H__
#define __GAME_ENTITY_H__

#include "cocos2d.h"
#include "DreamDialogue.h"

// 前向声明，避免循环引用
class Player;

class GameEntity : public cocos2d::Sprite
{
public:
    // 1. 设置/获取梦语
    void setDreamThought(const std::string& text) { _dreamThought = text; }
    std::string getDreamThought() const { return _dreamThought; }

    // =============================================================
    // 核心虚函数 (子类必须实现或重写)
    // =============================================================

    // A. 获取受击判定框
    virtual cocos2d::Rect getHitbox() const = 0;

    // B. 受到普通攻击 (统一接口：伤害值 + 攻击来源位置用于击退)
    // 哪怕罐子是一击碎，也可以忽略 damage 参数
    virtual void takeDamage(int damage, const cocos2d::Vec2& sourcePos) {
        // 默认行为：空
    }

    // C. 受到梦之钉攻击
    virtual void onDreamNailHit()
    {
        if (!_dreamThought.empty())
        {
            auto dialogue = DreamDialogue::create(_dreamThought);
            if (dialogue) {
                // 默认显示在头顶上方 60 像素
                dialogue->setPosition(this->getPosition() + cocos2d::Vec2(0, 60));

                // 添加到 Parent (即 GameLayer) 而不是 this，防止随本体一起消失
                if (this->getParent()) {
                    this->getParent()->addChild(dialogue, 200);
                    dialogue->show();
                }
            }
            cocos2d::log("Dream Nail Hit Entity: %s", _dreamThought.c_str());
        }
        else {
            cocos2d::log("Dream Nail Hit Entity (No Thought)");
        }
    }

    // 虚函数：检查是否存活/有效
    virtual bool isValidEntity() const { return true; }

protected:
    std::string _dreamThought;
};

#endif
