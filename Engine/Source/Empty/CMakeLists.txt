set(EngineEmptyDir "${EngineSourceDir}/Empty")

file(GLOB EmptyHeaderFiles CONFIGUE_DEPENDS "*.h")
file(GLOB EmptySourceFiles CONFIGUE_DEPENDS "*.cpp")

source_group(TREE ${EngineEmptyDir} FILES ${EmptyHeaderFiles} ${EmptySourceFiles})

# E3 空引擎验收：可执行体仅链接 XLEngineRuntime（引擎核心），刻意不链接 XLEngineGame，
# 验证引擎可脱离玩法内容独立构建并运行空关卡。
add_executable(XLEngineEmpty ${EmptyHeaderFiles} ${EmptySourceFiles})

add_compile_definitions("ENGINE_ROOT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# 注意：只链接 XLEngineRuntime，不含 XLEngineGame —— 这是"空引擎"验收的关键点
target_link_libraries(XLEngineEmpty PRIVATE XLEngineRuntime winmm)

target_include_directories(XLEngineEmpty PRIVATE
	"${EngineSourceDir}"
	"${EngineRuntimeDir}"
)

target_compile_definitions(XLEngineEmpty PRIVATE
	XL_PLATFORM_WINDOWS
	XL_DEBUG
	_CRT_SECURE_NO_WARNINGS
)