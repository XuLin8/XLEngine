#pragma once

#include <memory>

// Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
		/* Windows x64  */
		#define XL_PLATFORM_WINDOWS
	#else
		/* Windows x86  */
		#error "x86 Builds are not supported!"
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	 * so we must check all of them (in this order)
	 * to ensure that we're running on MAC
	 * and not some other Apple platform */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define XL_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define XL_PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
	#endif
/* We also have to check __ANDROID__ before __linux__
* since android is based on the linux kernel
* it has __linux__ defined */
#elif defined(__ANDROID__)
	#define XL_PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define XL_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif// End of platform detection

// DLL support
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
#endif // End of DLL support 

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
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}