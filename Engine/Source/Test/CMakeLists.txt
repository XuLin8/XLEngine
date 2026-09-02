set(EngineTestDir "${EngineSourceDir}/Test")

# 头less ECS 核心单元测试：仅链接 XLEngineRuntime（引擎核心，含有 ECS/Level/SceneSerializer），
# 不搭窗口/渲染环，也不链接任何 Game 玩法模块。
add_executable(XLEngineTests
	TestMain.cpp
	EcsTests.cpp
	EngineTest.h
)

target_link_libraries(XLEngineTests PRIVATE XLEngineRuntime winmm)

target_include_directories(XLEngineTests PRIVATE
	"${EngineSourceDir}"
	"${EngineRuntimeDir}"
)

target_compile_definitions(XLEngineTests PRIVATE
	XL_PLATFORM_WINDOWS
	XL_DEBUG
	_CRT_SECURE_NO_WARNINGS
	"ENGINE_ROOT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# 接入 CTest：ctest --test-dir build 即可运行 ECS 回归
enable_testing()
add_test(NAME EcsCoreTests COMMAND XLEngineTests)
set_tests_properties(EcsCoreTests PROPERTIES WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")