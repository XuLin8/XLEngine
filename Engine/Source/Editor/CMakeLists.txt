file(COPY ${EngineEditorDir}/Resources
DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(COPY ${EngineEditorDir}/imgui.ini
DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB EditorHeaderFiles CONFIGUE_DEPENDS "*.h" "Panels/*.h")
file(GLOB EditorSourceFiles CONFIGUE_DEPENDS "*.cpp" "Panels/*.cpp")

source_group(TREE ${EngineEditorDir} FILES ${EditorHeaderFiles} ${EditorSourceFiles})

add_executable(XLEngineEditor ${EditorHeaderFiles} ${EditorSourceFiles})

add_compile_definitions("ENGINE_ROOT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

target_link_libraries(XLEngineEditor PRIVATE XLEngineRuntime)

target_include_directories(XLEngineEditor PRIVATE
	"${EngineSourceDir}"
	"${EngineRuntimeDir}"
)

target_compile_definitions(XLEngineEditor PRIVATE
	XL_PLATFORM_WINDOWS
	XL_DEBUG
	_CRT_SECURE_NO_WARNINGS
)

if(MSVC)
	set_target_properties(
	XLEngineEditor PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${EngineEditorDir}")
endif()