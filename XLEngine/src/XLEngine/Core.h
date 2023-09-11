#pragma once

#ifdef XL_PLATFORM_WINDOWS
	#ifdef XL_BUILD_DLL
		#define XLENGINE_API __declspec(dllexport)
	#else
		#define XLENGINE_API __declspec(dllimport)
	#endif
#else
	#error XLEngine only supports Windows!
#endif // XL_PLATFORM_WINDOWS

#define BIT(x) (1 << x)

#define XL_ENABLE_ASSERTS 1
#ifdef XL_ENABLE_ASSERTS
	#define XL_ASSERT(x,...){if(!(x)) {XL_ERROR("Assertion Failed:{0}",__VA_ARGS__);__debugbreak();}}
	#define XL_CORE_ASSERT(x,...){if(!(x)) {XL_CORE_ERROR("Assertion Failed:{0}",__VA_ARGS__);__debugbreak();}}
#else
	#define XL_ASSERT(x,...)
	#define XL_CORE_ASSERT(x,...)
#endif