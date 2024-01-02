#pragma once

#include "Runtime/Core/Base.h"
#include "Runtime/Core/Log.h"
#include <filesystem>

#ifdef XL_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define XL_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { XL##type##ERROR(msg, __VA_ARGS__); XL_DEBUGBREAK(); } }
#define XL_INTERNAL_ASSERT_WITH_MSG(type, check, ...) XL_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define XL_INTERNAL_ASSERT_NO_MSG(type, check) XL_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", XL_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define XL_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define XL_INTERNAL_ASSERT_GET_MACRO(...) XL_EXPAND_MACRO( XL_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, XL_INTERNAL_ASSERT_WITH_MSG, XL_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define XL_ASSERT(...) XL_EXPAND_MACRO( XL_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define XL_CORE_ASSERT(...) XL_EXPAND_MACRO( XL_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define XL_ASSERT(...)
#define XL_CORE_ASSERT(...)
#endif