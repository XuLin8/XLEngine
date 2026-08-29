#include "xlpch.h"
#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace XLEngine 
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	namespace
	{
		// 保留最近的 N 次启动日志：XLEngine.log（当前）+ .1/.2/.3/.4（历史最近启动）。
		// 每次启动将现有文件顺延一位，丢到最旧的 .4。
		constexpr int kKeepLaunchLogs = 5;

		std::filesystem::path GetLogDir()
		{
			wchar_t buf[MAX_PATH];
			const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
			std::filesystem::path exe(n > 0 ? std::wstring(buf, n) : std::wstring(L"XLEngineEditor.exe"));
			return exe.parent_path() / "logs";
		}

		void RotateLaunchLogs(const std::filesystem::path& dir)
		{
			const int keep = kKeepLaunchLogs; // .log + .1/.2/.3/.4 = 共 5 份
			// 先删最旧的 .(keep-1)
			{
				std::error_code ec;
				std::filesystem::remove(dir / ("XLEngine." + std::to_string(keep - 1) + ".log"), ec);
			}
			// 从 .(keep-2) 递推到 .1，各自向后顺延一位
			for (int i = keep - 2; i >= 1; --i)
			{
				std::error_code ec;
				std::filesystem::path from = dir / ("XLEngine." + std::to_string(i) + ".log");
				if (std::filesystem::exists(from))
					std::filesystem::rename(from, dir / ("XLEngine." + std::to_string(i + 1) + ".log"), ec);
			}
			// 当前日志顺延为 .1
			{
				std::error_code ec;
				std::filesystem::path cur = dir / "XLEngine.log";
				if (std::filesystem::exists(cur))
					std::filesystem::rename(cur, dir / "XLEngine.1.log", ec);
			}
			std::error_code ec2;
			std::filesystem::create_directories(dir, ec2);
		}

		std::filesystem::path GetLogFilePath()
		{
			const std::filesystem::path dir = GetLogDir();
			RotateLaunchLogs(dir);
			return dir / "XLEngine.log";
		}
	}

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		try
		{
			// truncate=true：每次启动覆盖旧的日志，避免无限增长
			sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(GetLogFilePath().string(), true));
		}
		catch (...)
		{
			// 无法创建日志文件时仅保留控制台输出，不影响引擎运行
		}

		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_CoreLogger = std::make_shared<spdlog::logger>("XLEngine", sinks.begin(), sinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace); // 每行即时落盘，崩溃不丢最后一条

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);
	}
}