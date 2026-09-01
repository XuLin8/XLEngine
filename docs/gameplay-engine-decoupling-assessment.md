# 引擎与 Gameplay 解耦评估报告

> 目标：让引擎可以被任意游戏复用，gameplay 独立成模块，二者不再混用同一份代码库。
> 本报告只做盘点与拆分方案，不动任何引擎代码。

## 1. 结论先行

- **现状**：引擎与 gameplay **没有分开**，两者在同一个源码树（`Engine/Source`）、同一个可执行目标里编译运行。
- **致命耦合**：引擎核心类 `Level`、`World` 直接 `#include` 并依赖具体玩法类 `GameSystem`；`Level` 构造函数甚至硬编码 `new GameSystem(...)`。这意味着**引擎必须和玩法一起编译，无法脱离游戏运行**。
- **好消息**：`System` 基类、`GameMode` 基类、`World/Level` 扩展点（P4 成果）已经为拆分预留了正确接口，方向是通的。

## 2. 现状盘点：哪些代码属于引擎，哪些属于 gameplay

> 判定标准：**"通用、可被任意游戏复用" = 引擎；"这个 M2 游戏的专属规则/内容" = gameplay**。

### 2.1 内嵌在引擎里的 gameplay（应移出）

| 位置 | 内容 | 说明 |
|------|------|------|
| `Runtime/EcsFramework/System/Game/GameSystem.*` | M2 玩法循环：探索/收集光尘/点亮灯台/黎明 | 玩法逻辑，种子、坐标、规则全是本作专属 |
| `EditorLayer.cpp` (约 112–216 行) | 硬编码生成 地形/45棵树/25岩石/6遗迹，含 LCG 种子、坐标、变体 | **内容前端硬编码在工具层**，非数据驱动 |
| `Runtime/EcsFramework/Component/Prop/PropComponent.*` | 树木/岩石/遗迹的程序化散布 | 生成规则是本作专属 |
| `Runtime/EcsFramework/Component/Terrain/TerrainComponent.*` | 地形网格生成 | 技术是引擎能力，但"这一座岛"的形态属于内容 |
| `InputActionMapper::Get().LoadDefaultBindings()` | WASD/方向键 等玩法绑定 | 绑定定义是玩法专属，现固化在引擎输入层 |
| `GameMode/DefaultGameMode.*` | BeginPlay/EndPlay/Tick 钩子 | 类本身可作引擎默认，含的"运行规则"部分属 gameplay |

### 2.2 引擎暴露给 gameplay 的扩展点（应保留，是好东西）

| 位置 | 作用 |
|------|------|
| `System`（`System.h`） | `OnUpdateRuntime/OnUpdateEditor/OnRuntimeStart/OnRuntimeStop/OnRender3D` 虚接口，gameplay 系统从这里派生 |
| `GameMode`（`GameMode.h`） | `BeginPlay/EndPlay/Tick` 虚接口，gameplay 会话从这派生并注册 |
| `World`（持久关卡/运行副本、编辑↔运行态） | 通用世界容器，P4 已做好 |
| `Level` / `AssetRegistry` / URDF `.xld` / `SceneSerializer` | 场景与资产的通用数据层，gameplay 应通过它们加载内容 |

## 3. 六处硬耦合（拆分必须先切断）

1. `Level::Level()` 构造函数硬编码 `mSystems.push_back(new GameSystem(this))`
   → 引擎每次建关卡必然构造玩法系统，无法脱离。
2. `Level::GetGameSystem()` 返回 `GameSystem*`（`Level.h:46`）
   → 引擎核心类型依赖具体玩法类。
3. `World::GetGameSystem()` 返回 `GameSystem*`，且 `World.cpp` `#include "System/Game/GameSystem.h"`（`World.cpp:3,68`）
   → 世界容器直接依赖玩法类。
4. `EditorLayer` HUD 通过 `m_ActiveScene->GetGameSystem()` 读进度
   → 工具层依赖具体玩法查询接口。
5. `SceneSerializer` 序列化玩法组件
   → 引擎侧序列化需对玩法组件编解码，需走通用类型注册，避免逐个硬写。
6. `EditorLayer::OnAttach` 硬编码实体生成（种子/坐标/数量）
   → 内容写死工具层，未走 `.xl`/URDF 数据驱动。

> 依赖方向错误是核心：**底层（引擎）反向依赖了上层（玩法）**，违反六层单向依赖原则，也是"不能混用一个代码库"的最直接体现。

## 4. 目标架构

```
repo/
├─ Engine/                仅引擎：平台无关、可复用
│  └─ Runtime/…  World/Level/System/GameMode(基类)/AssetRegistry/URDF/…
├─ Game/                  独立 gameplay 模块（独立 CMake 目标，链接 Engine 库）
│  ├─ Systems/GameSystem.*      ← 从引擎 System 派生
│  ├─ Rules/GameRules.*         ← 从引擎 GameMode 派生，注册玩法规则
│  ├─ Components/PropRules...   ← 散布规则移入
│  ├─ Bindings/…                ← 玩法输入绑定注册
│  └─ Content/*.xl + *.xld      ← 场景/资产数据驱动，替代硬编码
└─ 可执行体（Editor + Runtime）  仅做装配：加载 Engine + Game 两库
```

**依赖方向必须单向：`Game → Engine`，engine 绝不反向 include 玩法。** 引擎只提供接口/查询 API；玩法通过注册（而非被引擎硬 new）进入关卡/世界。

## 5. 分阶段执行方案（每阶段构建+冒烟自检）

### 阶段 A：切断"引擎→玩法类"依赖
- `Level` 构造函数去掉 `new GameSystem`；改为通用系统注册 API（如 `RegisterSystem(System*)`）。
- 新增引擎侧通用访问：`Level::GetSystem<T>()` / 遍历系统，删除 `Level::GetGameSystem()`、`World::GetGameSystem()`。
- HUD 查询改由 Game 模块自己的访问器提供（`GameMode` 下引/`GameSession` 查询），不经引擎硬类型。
- **验收**：移除 GameSystem 后引擎仍能构建；冒烟通过。

### 阶段 B：新建 Game 模块（CMake）
- 顶层加 `Game/CMakeLists.txt`，目标 `GameModule`，`target_link_libraries(GameModule PRIVATE Engine)`。
- 把 gameplay 系统/规则/Bindings 移入 `Game/`，通过注册函数挂到 Level（如 `GameInstance::Start(GameMode*)`）。
- **验收**：构建 + 冒烟；Play 后玩法照常跑。

### 阶段 C：内容数据驱动化
- 把 `EditorLayer::OnAttach` 的硬编码生成改成由 `.xl` 场景 + URDF 资产加载，或移入 Game 模块的 spawner 读取配置。
- 玩法组件经统一类型注册供 `SceneSerializer` 通用编解码。
- **验收**：删掉编辑器硬编码后场景内容仍一致；冒烟通过。

### 阶段 D：清除侵入残留
- 玩法输入绑定改由 Game 模块注册进输入层（引擎输入层不再内置玩法绑定）。
- Terrain 拆分：引擎保留通用地形技术，具体"岛"布局归 Content。
- **验收**：不引入 Game 时引擎是"空引擎"，可构建运行空关卡。

## 6. 主要风险与对策

| 风险 | 对策 |
|------|------|
| 大改导致回归 | 每阶段独立提交 + 构建 / 冒烟 / 保留前 commit 可回退 |
| Level 去掉 GameSystem 后，运行时玩法没入口 | 由 Game 模块以 `GameMode` 注册系统；World.Play 走 GameMode 钩子 |
| 玩法组件序列化耦合 | 用统一类型注册表（可用 P2 反射/类型元数据打底）通用编解码 |
| 引擎反向 INC（循环依赖） | 严格单向 Game→Engine，引擎只给接口，不给具体玩法调用 |
| HUD 失去 GetGameSystem | 引擎提供通用查询 API，HUD 或 Game 侧做类型安全查询 |

## 7. 依赖既有成果
- P4 的 `World/GameMode` 扩展点正是为此次拆分铺设的接口；
- 建议在 **P2 反射系统（类型元数据）** 落地后再搬玩法组件，能让阶段 C 的通用序列化更稳；
- 若急于拆分可先做 A–B（代码层解耦），C–D（数据驱动）可与反射并行。

---
状态：评估完成，未改任何代码。待确认后再进入阶段 A 实施。