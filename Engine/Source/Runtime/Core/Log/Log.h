#pragma once
#include "Runtime/Core/Base/Base.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/bundled/format.h>
#include <filesystem>

namespace XLEngine
{
	class XLENGINE_API Log
	{
	public:
		static void Init();

		// 断言/致命错误弹窗：在交互式调试时供用户看到报错位置与信息
		static void ShowAssertDialog(const std::string& message);

		[[nodiscard]] inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		[[nodiscard]] inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// 调用点位置前缀（用于报错类日志，无需改动调用处即可定位源码）
#define XL_SOURCE_LOC() (std::filesystem::path(__FILE__).filename().string() + ":" + std::to_string(__LINE__))

// Core log macros
#define XL_CORE_TRACE(...)		::XLEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define XL_CORE_INFO(...)		::XLEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define XL_CORE_WARN(...)		::XLEngine::Log::GetCoreLogger()->warn("[{0}] {1}", XL_SOURCE_LOC(), fmt::format(__VA_ARGS__))
#define XL_CORE_ERROR(...)		::XLEngine::Log::GetCoreLogger()->error("[{0}] {1}", XL_SOURCE_LOC(), fmt::format(__VA_ARGS__))
#define XL_CORE_FATAL(...)		::XLEngine::Log::GetCoreLogger()->critical("[{0}] {1}", XL_SOURCE_LOC(), fmt::format(__VA_ARGS__))

// Client log macros
#define XL_TRACE(...)			::XLEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define XL_INFO(...)			::XLEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define XL_WARN(...)			::XLEngine::Log::GetClientLogger()->warn("[{0}] {1}", XL_SOURCE_LOC(), fmt::format(__VA_ARGS__))
#define XL_ERROR(...)			::XLEngine::Log::GetClientLogger()->error("[{0}] {1}", XL_SOURCE_LOC(), fmt::format(__VA_ARGS__))
#define XL_FATAL(...)			::XLEngine::Log::GetClientLogger()->critical("[{0}] {1}", XL_SOURCE_LOC(), fmt::format(__VA_ARGS__))