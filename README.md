# XLEngine
Integrating what I've learned

only support platform Windows

OpenGL 4.5

## 命名规范

### 命名法
统一采用Pascal命名法（文件夹、类名等），第三方库除外

CMakeLists.txt 的变量命名也采用 Pascal 命名法，比如：
```
set(ProjectRootDir "${CMAKE_CURRENT_SOURCE_DIR}")
```
这里变量 ProjectRootDir 采用 Pascal 命名法，与 CMAKE 自带变量区分（比如 CMAKE_CURRENT_SOURCE_DIR ）

### include 头文件顺序
首先include同级文件，其次是同Source文件，再次为第三方依赖（确保依赖顺序）
比如 Editor 中：
```
// 同级文件（同属于 Editor ）
#include "EditorLayer.h"

// 同Source文件（位于 Runtime 中）
#include <XLEngine.h>
#include <Runtime/Core/EntryPoint.h>	

// 第三方依赖
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
```
### 代码规范
XLEngine 的代码规范偏Unreal，可参考：
https://docs.unrealengine.com/4.27/zh-CN/ProductionPipelines/DevelopmentSetup/CodingStandard/

原则：
1. 尽量不使用下划线
1. 如果是类内部成员变量，在前面加小写字母m
1. 如果是bool类型变量，在前面加小写字母b（覆盖上一条，即类内部的bool类型成员变量只需要加b即可）
1. 类内成员统一放在类的最末尾（方法置于前）
1. 临时变量一律小写字母开头

## Getting Started
**1. Downloading the repository**
`git clone git@github.com:Xulin8/XLEngine.git`

**2. You can choose one of the following methods to build XLEngine:**

2.1 Run the Win-GenProjects.bat 

2.2 Run the following commands:
```
cd XLEngine
cmake -B build
cmake --build build --parallel 4
```
2.3 Visual Studio: Open Folder, then choose XLEngine folder)

## 编辑器操作（Editor Controls）

### 相机漫游（Roam Mode / FPS）
按 `F` 键，或 **Settings → Camera → Roam mode (FPS)** 勾选，可在场景中自由漫游预览：

| 按键 | 功能 |
| ---- | ---- |
| `W` / `A` / `S` / `D` | 前后左右移动（沿相机平面） |
| `空格` / `Q` | 上升 / 下降 |
| 鼠标右键（按住） | 视角旋转（FPS 式） |
| `Shift` | 加速（4 倍） |
| 滚轮 | 沿视线方向平移 |
| `F` | 退出漫游 |

漫游模式下自动禁用 gizmo 快捷键（`Q/W/E/R`），避免与移动键冲突；退出漫游后恢复 gizmo 操作。输入仅在视口悬停/聚焦时生效，不会误触其他面板。进入/退出漫游时视角无缝衔接。

### 轨道视角（Orbit Mode，默认）
| 按键 | 功能 |
| ---- | ---- |
| `Alt` + 左键（按住） | 旋转视角 |
| `Alt` + 中键（按住） | 平移 |
| `Alt` + 右键（按住） / 滚轮 | 缩放 |

### Gizmo 变换
- `W` / `E` / `R`：切换 平移 / 旋转 / 缩放 gizmo
- `Q`：取消 gizmo
- `Ctrl`（按住）：对齐吸附（平移/缩放 0.5，旋转 45°）

### M1 风格化视口
- 默认全屏水墨描边风格化输出
- **Settings → Diagnostic split view**：切换 2x2 诊断分屏（原始场景 / 边缘图 / 法线 / 深度）
- **Settings → Auto day-night cycle**：自动昼夜循环（约 33s 一圈）

## Credits
* hebohang [HEngine](https://github.com/hebohang/HEngine)
* Cherno [Hazel](https://github.com/TheCherno/Hazel)
* BoomingTech [Pilot](https://github.com/BoomingTech/Pilot)