#include "PlayerAnimator.h"
#include "SimpleAudioEngine.h"
#include "Config.h" // 需要读取路径配置

USING_NS_CC;
using namespace CocosDenshion;

PlayerAnimator::PlayerAnimator() : _owner(nullptr), _slashEffectSprite(nullptr), _focusEffectSprite(nullptr)
{
}

PlayerAnimator::~PlayerAnimator()
{
}

void PlayerAnimator::init(Sprite* owner)
{
    _owner = owner;

    // 1. 加载所有动画资源
    loadAllAnimations();


    // 2. 初始化刀光特效
    _slashEffectSprite = Sprite::create();
    _slashEffectSprite->setVisible(false);
    _owner->addChild(_slashEffectSprite, 10); // 加在主角身上

    // 3. 初始化凝聚特效
    _focusEffectSprite = Sprite::create();
    _focusEffectSprite->setPosition(_owner->getContentSize() / 2); // 居中
    _focusEffectSprite->setVisible(false);
    _focusEffectSprite->setBlendFunc(BlendFunc::ADDITIVE); // 叠加模式更亮
    _owner->addChild(_focusEffectSprite, 10);
}

void PlayerAnimator::loadAnim(const std::string& name, const std::string& format, int count, float delay)
{
    Vector<SpriteFrame*> frames;
    for (int i = 1; i <= count; i++) {
        std::string path = StringUtils::format(format.c_str(), i);
        auto sprite = Sprite::create(path);
        if (sprite) frames.pushBack(sprite->getSpriteFrame());
    }
    if (!frames.empty()) {
        auto anim = Animation::createWithSpriteFrames(frames, delay);
        _animations.insert(name, anim);
    }
}

void PlayerAnimator::loadAllAnimations()
{
    loadAnim("idle", Config::Path::PLAYER_IDLE, 9, 0.15f);
    loadAnim("run", Config::Path::PLAYER_RUN, 13, 0.15f);
    loadAnim("jump", Config::Path::PLAYER_JUMP, 6, 0.15f);
    loadAnim("fall", Config::Path::PLAYER_FALL, 6, 0.15f);
    loadAnim("slash", Config::Path::PLAYER_SLASH, 6, 0.04f);
    loadAnim("damage", Config::Path::PLAYER_DAMAGED, 8, 0.06f);
    loadAnim("lookup", Config::Path::PLAYER_LOOKUP, 6, 0.1f);
    loadAnim("lookdown", Config::Path::PLAYER_LOOKDOWN, 6, 0.1f);
    loadAnim("slash_up", Config::Path::PLAYER_UPSLASH, 6, 0.04f);
    loadAnim("slash_down", Config::Path::PLAYER_DOWNSLASH, 6, 0.04f);
    loadAnim("death", Config::Path::PLAYER_DEATH, 20, 0.08f);

    // 凝聚
    loadAnim("focus_loop", Config::Path::PLAYER_FOCUS_LOOP, 7, 0.08f);
    loadAnim("focus_get", Config::Path::PLAYER_FOCUS_GET, 11, 0.06f);
    loadAnim("focus_end", Config::Path::PLAYER_FOCUS_LOOP, 3, 0.08f);

    // 特效
    loadAnim("slash_effect", Config::Path::PLAYER_SLASH_EFFECT, 6, 0.04f);
    loadAnim("slash_up_effect", Config::Path::PLAYER_UP_SLASH_EFFECT, 6, 0.04f);
    loadAnim("slash_down_effect", Config::Path::PLAYER_DOWN_SLASH_EFFECT, 6, 0.04f);

    // 凝聚特效
    loadAnim("effect_focus_loop", Config::Path::EFFECT_FOCUS_LOOP, 13, 0.06f);
    loadAnim("effect_focus_end", Config::Path::EFFECT_FOCUS_END, 2, 0.08f);

    //施法两段动画
    loadAnim("cast_antic", Config::Path::PLAYER_CAST_ANTIC, 3, 0.05f);
    loadAnim("cast_release", Config::Path::PLAYER_CAST_RELEASE, 6, 0.06f);

	// 梦之钉      
    loadAnim("dream_nail_charge", Config::Path::DREAM_NAIL_CHARGE, 7, 0.1f);
    loadAnim("dream_nail_slash", Config::Path::DREAM_NAIL_SLASH, 15, 0.04f);
}

void PlayerAnimator::preloadSounds()
{
    auto audio = SimpleAudioEngine::getInstance();

    // 1. 背景音乐
    audio->preloadBackgroundMusic(Config::Audio::BGM_DIRTMOUTH);

    // 2. 动作音效
    audio->preloadEffect(Config::Audio::HERO_JUMP);
    audio->preloadEffect(Config::Audio::HERO_LAND_SOFT);
    audio->preloadEffect(Config::Audio::HERO_LAND_HARD);
    audio->preloadEffect(Config::Audio::HERO_RUN);
    audio->preloadEffect(Config::Audio::HERO_UNSHEATH);

    // 3. 战斗音效
    audio->preloadEffect(Config::Audio::HERO_DAMAGE);
    audio->preloadEffect(Config::Audio::HERO_DEATH);

    // 4. 攻击音效
    audio->preloadEffect(Config::Audio::SWORD_1);
    audio->preloadEffect(Config::Audio::SWORD_2);
    audio->preloadEffect(Config::Audio::SWORD_3);

    // 5. 技能音效
    audio->preloadEffect(Config::Audio::FOCUS_CHARGE);
    audio->preloadEffect(Config::Audio::FOCUS_HEAL);

    CCLOG("[PlayerAnimator] Audio Assets Preloaded Successfully from Config!");
}
void PlayerAnimator::playAnimation(const std::string& animName)
{
    if (!_owner) return;

    // 停止之前的动画 (Tag 101)
    _owner->stopActionByTag(101);

    if (_animations.find(animName) == _animations.end()) {
        CCLOG("Error: Animation '%s' not found!", animName.c_str());
        return;
    }

    auto anim = _animations.at(animName);
    Action* action = nullptr;

    // 判断是循环播放还是单次播放
    if (animName == "idle" || animName == "run" || 
        animName == "focus_loop"||animName=="jump"||
        animName=="fall" || animName == "dream_nail_charge")
    {
        action = RepeatForever::create(Animate::create(anim));
    }
    else
    {
        action = Animate::create(anim);
    }

    action->setTag(101);
    _owner->runAction(action);
}

// =======================
// 特效逻辑
// =======================

void PlayerAnimator::playAttackEffect(int dir, bool facingRight)
{
    if (!_slashEffectSprite) return;

    _slashEffectSprite->stopAllActions();
    _slashEffectSprite->setVisible(true);
    _slashEffectSprite->setFlippedX(facingRight);

    Size size = _owner->getContentSize();
    Vec2 center = Vec2(size.width / 2, 90);
    std::string animName = "slash_effect";
    Vec2 offset = Vec2::ZERO;

    if (dir == 1) { // Up
        animName = "slash_up_effect";
        offset = Vec2(50, 80);
    }
    else if (dir == -1) { // Down
        animName = "slash_down_effect";
        offset = Vec2(50, -70);
    }
    else { // Horizontal
        offset = Vec2(facingRight ? 20 : 40, 0);
    }

    _slashEffectSprite->setPosition(center + offset);

    auto anim = _animations.at(animName);
    if (anim) {
        auto seq = Sequence::create(
            Animate::create(anim),
            CallFunc::create([this]() { _slashEffectSprite->setVisible(false); }),
            nullptr
        );
        _slashEffectSprite->runAction(seq);
    }
}

void PlayerAnimator::startFocusEffect()
{
    if (!_focusEffectSprite) return;
    _focusEffectSprite->setVisible(true);
    _focusEffectSprite->stopAllActions();

    auto anim = _animations.at("effect_focus_loop");
    if (anim) _focusEffectSprite->runAction(RepeatForever::create(Animate::create(anim)));
}

void PlayerAnimator::stopFocusEffect()
{
    if (!_focusEffectSprite) return;
    _focusEffectSprite->stopAllActions();
    _focusEffectSprite->setVisible(false);
}

void PlayerAnimator::playFocusEndEffect()
{
    if (!_focusEffectSprite) return;
    _focusEffectSprite->setVisible(true);
    _focusEffectSprite->stopAllActions();

    auto anim = _animations.at("effect_focus_end");
    if (anim) {
        auto seq = Sequence::create(
            Animate::create(anim),
            CallFunc::create([this]() { _focusEffectSprite->setVisible(false); }),
            nullptr
        );
        _focusEffectSprite->runAction(seq);
    }
}