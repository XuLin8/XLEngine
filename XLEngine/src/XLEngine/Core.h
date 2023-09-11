#pragma once
#include "xlpch.h"

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