#ifndef __JAR_H__
#define __JAR_H__
#include "GameEntity.h" // 引入基类
#include "cocos2d.h"

USING_NS_CC;

class Jar : public GameEntity
{
public:
    static Jar* create(const std::string& jarImage, const Vec2& position);
    
    virtual bool init(const std::string& jarImage, const Vec2& position);
    
    // 获取碰撞箱
    virtual cocos2d::Rect getHitbox() const override;

    // 【重要】实现罐子特有的梦之钉反应
    virtual void onDreamNailHit() override;

    // 获取顶部平台碰撞箱（用于站立）
    Rect getTopPlatformBox() const;
    
    // 【适配】实现基类接口 (忽略 damage 和 sourcePos，因为罐子一碰就碎)
    virtual void takeDamage(int damage, const cocos2d::Vec2& sourcePos) override
    {
        this->takeDamage(); 
    }

    Rect getCollisionBox() const;

    // 受到攻击
    void takeDamage();
    
    // 检查是否已被摧毁
    bool isDestroyed() const { return _isDestroyed; }

    // ==========================================
    // 幼虫心声接口
    // ==========================================
    void setDreamThought(const std::string& text) { _dreamThought = text; }
    std::string getDreamThought() const { return _dreamThought; }

    // =========================================================
    // 静态工具函数：一键创建三个谜题罐子
    // 参数 parent:  要把罐子加到的场景 (传入 HelloWorld 的 this)
    // 参数 outList: 要把罐子存到的列表 (传入 HelloWorld 的 _jars)
    // =========================================================
    static void setupPuzzleJars(Node* parent, Vector<Jar*>& outList);

    virtual bool isValidEntity() const override { return !_isDestroyed; }

private:
    Sprite* _jarSprite;      // 罐子精灵
    Sprite* _grubSprite;     // 幼虫精灵
    bool _isDestroyed;       // 是否已被摧毁
    bool _isInvincible;
	int _health;			   // 罐子生命值
    // 存储心声文字
    std::string _dreamThought;

    // 播放幼虫附着动画（循环）
    void playGrubAttachAnimation();
    
    // 播放幼虫释放动画（一次）
    void playGrubFreeAnimation();
    
    // 罐子破碎后的清理
    void onJarBroken();

    // 保存回调函数
    std::function<void()> _onBrokenCallback;
};

#endif // __JAR_H__
