<div align="center">

# 🗡️ Hollow Knight Remake

### 基于 Cocos2d-x 的《空洞骑士》复刻版

![](C:\Users\徐千顺\AppData\Roaming\marktext\images\2026-04-21-19-26-10-image.png)

![](C:\Users\徐千顺\AppData\Roaming\marktext\images\2026-04-21-19-26-55-image.png)

![](C:\Users\徐千顺\AppData\Roaming\marktext\images\2026-04-21-19-27-24-image.png)

一款使用 **C++ 与 Cocos2d-x** 从零实现的 2D 横版动作冒险游戏，复刻自 Team Cherry 的经典作品《Hollow Knight》。项目涵盖角色状态机、敌人 AI、解谜机制、Boss 战与 Android 跨平台移植。

</div>

---

## 📖 目录

- [项目概览](#-项目概览)
- [核心特性](#-核心特性)
- [技术栈](#-技术栈)
- [系统架构](#-系统架构)
- [设计模式](#-设计模式)
- [关卡与玩法](#-关卡与玩法)
- [代码亮点](#-代码亮点)
- [Android 移植](#-android-移植)
- [构建与运行](#-构建与运行)
- [项目结构](#-项目结构)
- [团队协作](#-团队协作)
- [致谢与声明](#-致谢与声明)

---

## 🎮 项目概览

本项目为课程期末设计作品。除 Cocos2d-x 开源引擎及音美素材外，**所有代码均为原创**，严格遵循学术诚信规范。

| 项目属性    | 内容          |
|:------- |:----------- |
| 🎯 项目类型 | 2D 横版动作冒险游戏 |
| ⏱️ 开发周期 | 单学期课程设计     |
| 🎨 美术素材 | 开源素材 + 原作致敬 |
| 💻 代码原创 | ✅ 100% 原创   |

---

## ✨ 核心特性

- 🕹️ **精细的角色控制** — 基于状态机的丝滑操作手感，支持奔跑、跳跃、攻击、上/下劈、施法、凝聚、受击等 10+ 角色状态
- 👾 **多样化敌人 AI** — 巡逻型地面敌人、追击型僵尸、飞行型 Buzzer 等，采用独立有限状态机驱动
- 🔍 **梦之钉解谜系统** — 基于 Lambda 事件分发的交互解谜玩法
- 👑 **多阶段 Boss 战** — 跳跃砸地、火球弹幕、范围冲击波等复合技能机制
- 🎨 **组件化设计** — 角色逻辑拆分为 Stats / Animator / State 组件，高内聚低耦合
- 📱 **跨平台支持** — 一套代码同时支持 Windows PC 与 Android 移动端
- 🚀 **60 FPS 流畅体验** — AABB 碰撞优化、纹理管理、对象池等多项性能优化

---

## 🛠 技术栈

```text
Engine     │  Cocos2d-x (C++ 版)
Language   │  C++14
IDE        │  Visual Studio 2022
Platform   │  Windows (x86/x64) + Android (ARM)
Build      │  VS MSBuild / Android NDK
VCS        │  Git (Feature Branch Workflow)
```

---

## 🏗 系统架构

项目采用面向对象 + 组件化的混合架构，核心类层次如下：

```
Scene (游戏场景)
│
├── Player (主角控制器)
│   ├── PlayerState  ────── 抽象状态基类
│   │   ├── StateIdle       待机
│   │   ├── StateRun        奔跑
│   │   ├── StateJump       跳跃
│   │   ├── StateSlash      攻击
│   │   ├── StateCast       施法
│   │   ├── StateFocus      凝聚回血
│   │   └── ...             (上劈/下劈/受击/死亡)
│   ├── PlayerStats  ────── 数值组件（血量、魂量）
│   └── PlayerAnimator ──── 动画与音效组件
│
├── GameEntity (实体基类)
│   ├── Enemy               地面敌人
│   ├── Zombie              追击型僵尸
│   ├── Buzzer              飞行敌人
│   ├── Jar                 可破坏罐子
│   └── Spike               尖刺陷阱
│
├── Boss (关底 BOSS)
│
└── HUDLayer (UI 界面层)
```

---

## 🎯 设计模式

### 1. 状态机模式 (State Pattern)

角色行为通过独立的状态类驱动，避免臃肿的 `switch-case`：

```cpp
class PlayerState {
public:
    virtual void enter(Player* player) = 0;            // 进入状态
    virtual void update(Player* player, float dt) = 0; // 每帧更新
    virtual void exit(Player* player) = 0;             // 退出状态
};
```

每新增一个角色动作，只需新建一个子类，无需修改 `Player` 核心逻辑，完美契合 **开闭原则**。

### 2. 工厂模式 (Factory Pattern)

所有游戏对象均通过静态 `create()` 方法托管给引擎，避免内存泄漏：

```cpp
Player* Player::create(const std::string& filename) {
    Player* player = new (std::nothrow) Player();
    if (player && player->init()) {
        player->autorelease();   // 引擎自动管理生命周期
        return player;
    }
    CC_SAFE_DELETE(player);
    return nullptr;
}
```

### 3. 组件模式 (Component Pattern)

`Player` 类拆分为多个职责单一的组件：

| 组件               | 职责             |
|:---------------- |:-------------- |
| `PlayerStats`    | 血量、魂量、生命周期数值管理 |
| `PlayerAnimator` | 动画播放、特效与音效触发   |
| `PlayerState`    | 行为逻辑与状态转移      |

---

## 🗺 关卡与玩法

### Level 1 · 基础战斗

**核心机制**：敌人 AI 与 AABB 碰撞检测

- **Enemy 巡逻 AI**：`PATROL → ALERT → ATTACK → DEAD` 状态流转
- **Zombie 追击 AI**：基于距离检测的主动追击

```cpp
bool isPlayerInChaseRange(const Vec2& playerPos) {
    float distance = abs(playerPos.x - this->getPositionX());
    return distance <= _chaseRange;
}
```

- **Buzzer 飞行 AI**：向量合成实现垂直悬浮 + 水平追踪的复合运动
- **碰撞检测**：高效 AABB 算法

```cpp
bool intersects = (rect1.getMaxX() >= rect2.getMinX() &&
                   rect1.getMinX() <= rect2.getMaxX() &&
                   rect1.getMaxY() >= rect2.getMinY() &&
                   rect1.getMinY() <= rect2.getMaxY());
```

### Level 2 · 梦之钉解谜

**核心机制**：基于 Lambda 的事件分发与多态调用

```cpp
auto handleDreamHit = [&](GameEntity* entity) {
    if (entity && entity->isValidEntity()) {
        if (_player->getDreamNailHitbox().intersectsRect(entity->getHitbox())) {
            entity->onDreamNailHit();           // 多态触发梦语
            _player->setDreamNailActive(false);
        }
    }
};
```

- 三个罐子分别绑定不同梦语文本
- 通过 Tag 系统标识正确目标（Tag=888）
- 破坏正确罐子后动态生成 "复仇之魂" 道具

### Level 3 · Boss 战

**核心机制**：多阶段技能 + 回调驱动的弹幕系统

```cpp
_boss->setFireballCallback([this](const Vec2& pos) {
    auto fireball = FKFireball::create("boss/rampageAttack/fk-fireball.png");
    if (fireball) {
        fireball->setPosition(pos);
        _gameLayer->addChild(fireball, 7);
    }
});
```

Boss 三大核心技能：

- 🦘 **跳跃砸地** — 定时器驱动的区间攻击
- 🔥 **火球弹幕** — 回调动态生成的投射物
- 💥 **地面冲击波** — 精确 Hitbox 判定的范围伤害

---

## 💎 代码亮点

### 内存安全管理

严格遵循 Cocos2d-x 的引用计数机制，析构时全面清理：

```cpp
Player::~Player() {
    CC_SAFE_DELETE(_stats);
    CC_SAFE_DELETE(_animator);
    this->stopAllActions();           // 停止所有动作
    this->unscheduleAllCallbacks();   // 取消所有定时器
}
```

容器遍历时对无效指针进行安全校验：

```cpp
if (!jar || jar->getReferenceCount() == 0) {
    it = _jars.erase(it);   // 安全移除
    continue;
}
```

### 现代 C++ 特性

**Lambda 数据绑定**

```cpp
_stats->onHealthChanged = [this](int cur, int max) {
    if (_onHealthChanged) _onHealthChanged(cur, max);
};

enemy->setOnDeathCallback([=]() {
    if (_player) {
        _player->gainSoulOnKill();
        CCLOG("Soul gained from Enemy!");
    }
});
```

**STL 与类型推导**

```cpp
std::vector<cocos2d::Rect> _groundRects;

auto enemy = dynamic_cast<Enemy*>(_gameLayer->getChildByTag(999));
if (enemy != nullptr) {
    enemy->update(dt);
}
```

**迭代器安全删除**

```cpp
for (auto it = _jars.begin(); it != _jars.end(); ) {
    if (/* 满足移除条件 */) {
        it = _jars.erase(it);
    } else {
        ++it;
    }
}
```

---

## 📱 Android 移植

项目成功移植至 Android 平台，解决了三大关键技术问题：

| 问题         | 解决方案                                             |
|:---------- |:------------------------------------------------ |
| **分辨率自适应** | 基于 Cocos2d-x 的 Design Resolution 机制统一坐标系，支持多屏幕尺寸 |
| **触摸控制映射** | 设计虚拟按键层，将触摸事件映射为原键盘输入，保持逻辑层零改动                   |
| **JNI 集成** | 配置 Android NDK 环境，处理 Java 与 C++ 的生命周期桥接          |

### 性能优化策略

- **纹理压缩** — 选用 Android 原生支持的纹理格式降低显存占用
- **音频适配** — 适配 Android 音频子系统，背景音乐与音效分轨管理
- **资源加载** — 针对移动端内存限制优化按需加载策略

---

## 🚀 构建与运行

### Windows (PC)

```bash
# 1. 克隆仓库
git clone https://github.com/<your-repo>/hollow-knight-remake.git
cd hollow-knight-remake

# 2. 安装 Cocos2d-x (v3.17+)
#    参考: https://docs.cocos.com/cocos2d-x/manual/

# 3. 使用 Visual Studio 2022 打开
#    proj.win32/HollowKnight.sln

# 4. 选择 Release | x64，按 F5 编译运行
```

### Android

```bash
# 1. 安装 Android NDK 与 SDK
# 2. 进入 Android 项目目录
cd proj.android

# 3. 构建 APK
./gradlew assembleRelease

# 4. APK 输出位于
#    proj.android/app/build/outputs/apk/release/
```

---

## 📁 项目结构

```
hollow-knight-remake/
├── Classes/                 # C++ 核心代码
│   ├── Player/              # 主角相关（Player, PlayerState, Stats, Animator）
│   ├── Enemy/               # 敌人系统
│   ├── Boss/                # Boss 与技能
│   ├── Scenes/              # 关卡场景
│   ├── UI/                  # HUD 与菜单
│   └── Utils/               # 工具类
├── Resources/               # 游戏资源
│   ├── sprites/             # 角色/敌人/场景贴图
│   ├── audio/               # BGM 与音效
│   └── fonts/               # 字体文件
├── proj.win32/              # Windows 工程文件
├── proj.android/            # Android 工程文件
└── README.md
```

---

## 👥 团队协作

项目采用 **Feature Branch** 工作流，由 2 人团队协同开发：

- 🌿 `main` — 稳定可发布版本
- 🌿 `feature/*` — 功能开发分支
- 📝 **提交规范** — 每次 commit 附带清晰功能描述
- 👀 **代码审查** — 重要功能合并前进行 Code Review
- 📦 **模块化分工** — 降低代码冲突，提升协作效率

| 成员  | 主要负责模块                  |
|:--- |:----------------------- |
| 徐千顺 | 主角状态机、战斗系统、UI           |
| 赵晴  | 敌人 AI、Boss 战、Android 移植 |

---

## 🙏 致谢与声明

### 开源致谢

- [Cocos2d-x](https://www.cocos.com/) — 优秀的开源跨平台游戏引擎
- 《Hollow Knight》原作 Team Cherry — 提供了灵感来源

### 学术声明

> 本项目为学术课程设计作品，仅用于学习交流，**不用于任何商业用途**。所有代码均为团队原创，美术与音效资源来自开源素材或原作致敬性质的学习参考。

### 技术收获

通过本项目，我们完整实践了游戏开发全流程：**架构设计 → 模块实现 → 性能优化 → 跨平台发布**，在状态机、内存管理、事件分发、跨平台移植等方面积累了扎实的工程经验。

<div align="center">

**⭐ 如果你喜欢这个项目，欢迎 Star 支持！**

</div>
