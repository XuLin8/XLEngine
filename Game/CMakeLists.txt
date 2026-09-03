set(GameSourceDir "${ProjectRootDir}/Game/Source")

# 仅收集 Game/Source（玩法库），避免把独立可执行体 Game/App 一并编入静态库
file(GLOB_RECURSE GameHeaderFiles CONFIGURE_DEPENDS "${GameSourceDir}/*.h")
file(GLOB_RECURSE GameSourceFiles CONFIGURE_DEPENDS "${GameSourceDir}/*.cpp")

source_group(TREE "${GameSourceDir}" FILES ${GameHeaderFiles} ${GameSourceFiles})

# Game 模块：独立于引擎的 gameplay 库。依赖方向严格单向 Game → Engine，
# 引擎（XLEngineRuntime）绝不反向依赖本库。
add_library(XLEngineGame STATIC ${GameHeaderFiles} ${GameSourceFiles})

target_include_directories(XLEngineGame PRIVATE
	"${ProjectRootDir}/Engine/Source"        # 以 "Runtime/..." 访问引擎头文件
	"${ProjectRootDir}/Engine/Source/Runtime" # 以 "xlpch.h" 预编译头
	"${GameSourceDir}"                       # 模块内引用，如 "Systems/GameSystem.h"、"GameModule.h"
)

# winmm（PlaySoundW/WinMM 音频）PUBLIC 传播到游戏可执行体，且须排在 XLEngineRuntime 之后避免 --as-needed 丢弃
target_link_libraries(XLEngineGame PUBLIC XLEngineRuntime winmm)

target_compile_definitions(XLEngineGame PRIVATE
	XL_PLATFORM_WINDOWS
	XL_DEBUG
)

# ============ 独立游戏可执行体 ============
# 打包目标：链接 XLEngineRuntime(经 Game) + XLEngineGame 玩法，无编辑器 UI，
# 启动即进入运行时游玩关卡（立项文档 P2-1「独立运行程序」）。产出于 build/bin/XLEngineGameApp.exe。
set(GameAppDir "${ProjectRootDir}/Game/App")

file(GLOB GameAppHeaders CONFIGURE_DEPENDS "${GameAppDir}/*.h")
file(GLOB GameAppSources CONFIGURE_DEPENDS "${GameAppDir}/*.cpp")

source_group(TREE "${GameAppDir}" FILES ${GameAppHeaders} ${GameAppSources})

add_executable(XLEngineGameApp ${GameAppHeaders} ${GameAppSources})

target_include_directories(XLEngineGameApp PRIVATE
	"${ProjectRootDir}/Engine/Source"         # 以 "XLEngine.h" 引擎伞头
	"${ProjectRootDir}/Engine/Source/Runtime" # 以 "Runtime/..." / "xlpch.h" 访问
	"${GameSourceDir}"                        # 以 "GameModule.h"、"Systems/..." 访问玩法
	"${GameAppDir}"
)

target_link_libraries(XLEngineGameApp PRIVATE XLEngineGame)

target_compile_definitions(XLEngineGameApp PRIVATE
	XL_PLATFORM_WINDOWS
	XL_DEBUG
	_CRT_SECURE_NO_WARNINGS
)