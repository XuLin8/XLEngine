set(GameSourceDir "${ProjectRootDir}/Game/Source")

file(GLOB_RECURSE GameHeaderFiles CONFIGURE_DEPENDS "*.h")
file(GLOB_RECURSE GameSourceFiles CONFIGURE_DEPENDS "*.cpp")

source_group(TREE "${GameSourceDir}" FILES ${GameHeaderFiles} ${GameSourceFiles})

# Game 模块：独立于引擎的 gameplay 库。依赖方向严格单向 Game → Engine，
# 引擎（XLEngineRuntime）绝不反向依赖本库。
add_library(XLEngineGame STATIC ${GameHeaderFiles} ${GameSourceFiles})

target_include_directories(XLEngineGame PRIVATE
	"${ProjectRootDir}/Engine/Source"        # 以 "Runtime/..." 访问引擎头文件
	"${ProjectRootDir}/Engine/Source/Runtime" # 以 "xlpch.h" 预编译头
	"${GameSourceDir}"                       # 模块内引用，如 "Systems/GameSystem.h"、"GameModule.h"
)

target_link_libraries(XLEngineGame PUBLIC XLEngineRuntime)

target_compile_definitions(XLEngineGame PRIVATE
	XL_PLATFORM_WINDOWS
	XL_DEBUG
)