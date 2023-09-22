#pragma once

#include <memory>

#ifdef XL_PLATFORM_WINDOWS
#if XL_DYNAMIC_LINK
	#ifdef XL_BUILD_DLL
		#define XLENGINE_API __declspec(dllexport)
	#else
		#define XLENGINE_API __declspec(dllimport)
	#endif
#else
	#define XLENGINE_API
#endif
#else
	#error XLEngine only supports Windows!
#endif // XL_PLATFORM_WINDOWS

#ifdef XL_DEBUG
	#define XL_ENABLE_ASSERTS
#endif

#ifdef XL_ENABLE_ASSERTS
	#define XL_ASSERT(x,...){if(!(x)) {XL_ERROR("Assertion Failed:{0}",__VA_ARGS__);__debugbreak();}}
	#define XL_CORE_ASSERT(x,...){if(!(x)) {XL_CORE_ERROR("Assertion Failed:{0}",__VA_ARGS__);__debugbreak();}}
#else
	#define XL_ASSERT(x,...)
	#define XL_CORE_ASSERT(x,...)
#endif

#define BIT(x) (1 << x)

//将事件的处理函数（如OnEvent）与其他函数（如 m_Window->SetEventCallback）绑定起来
#define XL_BIND_EVENT_FN(x) [this](auto&&... args) -> decltype(auto) { return this->x(std::forward<decltype(args)>(args)...); }

namespace XLEngine
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

}