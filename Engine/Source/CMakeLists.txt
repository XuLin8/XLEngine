set(ThirdPartyDir "${EngineSourceDir}/ThirdParty")
set(EngineRuntimeDir "${EngineSourceDir}/Runtime")
set(EngineEditorDir "${EngineSourceDir}/Editor")

add_subdirectory(ThirdParty)
add_subdirectory(Runtime)
add_subdirectory(Editor)
add_subdirectory(Empty)
add_subdirectory(Test)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT XLEngineEditor)