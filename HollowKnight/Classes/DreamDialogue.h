#ifndef __DREAM_DIALOGUE_H__
#define __DREAM_DIALOGUE_H__

#include "cocos2d.h"

class DreamDialogue : public cocos2d::Node {
public:
    static DreamDialogue* create(const std::string& text);
    virtual bool init(const std::string& text);

    // 执行显示流程：出现 -> 停留 -> 消失 -> 自毁
    void show();

private:
    cocos2d::Sprite* _bg;
    cocos2d::Label* _label;
};

#endif