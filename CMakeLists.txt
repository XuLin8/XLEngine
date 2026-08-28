cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

project(XLEngine LANGUAGES C CXX)

# GCC >= 13 起对标准头文件的自包含检查更严格，部分第三方源码
# （yaml-cpp / assimp / ...）直接使用 uint8_t 等类型但未包含 <cstdint>。
# 强制预包含 <cstdint> 以兼容 GCC/Clang 的严格模式（仅对 C++ 源生效）。
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-include;cstdint>")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(ProjectRootDir "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(Engine)