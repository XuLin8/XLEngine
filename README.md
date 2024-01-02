# XLEngine
Integrating what I've learned

## 命名法
统一采用Pascal命名法（文件夹、类名等），第三方库除外

CMakeLists.txt 的变量命名也采用 Pascal 命名法，比如：
```
set(ProjectRootDir "${CMAKE_CURRENT_SOURCE_DIR}")
```
这里变量 ProjectRootDir 采用 Pascal 命名法，与 CMAKE 自带变量区分（比如 CMAKE_CURRENT_SOURCE_DIR ）

## include 头文件顺序
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

# Getting Started
**1. Downloading the repository**
`git@github.com:hebohang/XLEngine.git`
**2. You can choose one of the following methods to build XLEngine:**
2.1 Run the Win-GenProjects.bat 
2.2 
```
cd XLEngine
cmake -B build
cmake --build build --parallel 4
```