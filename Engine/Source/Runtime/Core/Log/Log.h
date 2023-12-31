#pragma once
#include "Runtime/Core/Base/Base.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace XLEngine
{
	class XLENGINE_API Log
	{
	public:
		static void Init();

		[[nodiscard]] inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		[[nodiscard]] inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define XL_CORE_TRACE(...)		::XLEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define XL_CORE_INFO(...)		::XLEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define XL_CORE_WARN(...)		::XLEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define XL_CORE_ERROR(...)		::XLEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define XL_CORE_FATAL(...)		::XLEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros			  
#define XL_TRACE(...)			::XLEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define XL_INFO(...)			::XLEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define XL_WARN(...)			::XLEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define XL_ERROR(...)			::XLEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define XL_FATAL(...)			::XLEngine::Log::GetClientLogger()->critical(__VA_ARGS__)