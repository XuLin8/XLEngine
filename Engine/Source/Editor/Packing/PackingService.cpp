#include "xlpch.h"
#include "PackingService.h"

#include "Runtime/Resource/ConfigManager/ConfigManager.h"
#include "Runtime/Core/Log/Log.h"

#include <filesystem>
#include <stdexcept>
#include <windows.h>
#include <shellapi.h>

namespace XLEngine
{
	namespace
	{
		// 递归拷贝目录（跳过.git / .gitignore / CMake / *.obj / *.a / build 等中间产物）
		bool CopyRecursive(const std::filesystem::path& srcDir, const std::filesystem::path& dstDir, bool overwrite, std::vector<std::string>& outLog)
		{
			std::error_code ec;
			if (!std::filesystem::exists(srcDir))
			{
				outLog.push_back("WARN: 源目录不存在：" + srcDir.string() + " — 跳过");
				return true;
			}

			std::filesystem::create_directories(dstDir, ec);
			if (ec)
			{
				outLog.push_back("ERROR: 创建输出目录失败：" + dstDir.string() + " (" + ec.message() + ")");
				return false;
			}

			for (auto& entry : std::filesystem::directory_iterator(srcDir))
			{
				const auto& p = entry.path();
				auto filename = p.filename().string();

				// 跳过构建与版本控制产物（这些不需要打到游戏包里）
				if (filename == ".git" || filename == ".gitignore" || filename == "build"
					|| filename == "CMakeFiles" || filename == "bin" || filename == "*.obj")
					continue;
				if (p.extension() == ".obj" || p.extension() == ".a" || p.extension() == ".lib")
					continue;

				auto dst = dstDir / p.relative_path();
				if (entry.is_directory())
				{
					if (!CopyRecursive(p, dst, overwrite, outLog))
						return false;
				}
				else
				{
					bool doCopy = overwrite || !std::filesystem::exists(dst);
					if (!doCopy)
					{
						outLog.push_back("SKIP: " + p.string() + " → 已存在（未覆盖）");
						continue;
					}
					std::filesystem::copy_file(p, dst, std::filesystem::copy_options::overwrite_existing, ec);
					if (ec)
					{
						outLog.push_back("ERROR: 拷贝文件失败：" + p.string() + " → " + dst.string() + " (" + ec.message() + ")");
						return false;
					}
					outLog.push_back("COPY: " + p.string() + " → " + dst.string());
				}
			}
			return true;
		}
	}

	std::vector<std::string> PackingService::DiscoverExeCandidates()
	{
		std::vector<std::string> candidates;
		const auto binRoot = std::filesystem::path(ConfigManager::GetInstance().GetRootFolder());

		for (auto& entry : std::filesystem::directory_iterator(binRoot))
		{
			if (entry.path().extension() == ".exe")
				candidates.push_back(entry.path().filename().string());
		}
		return candidates;
	}

	bool PackingService::HasExe(const std::string& name)
	{
		const auto binRoot = std::filesystem::path(ConfigManager::GetInstance().GetRootFolder());
		return std::filesystem::exists(binRoot / name);
	}

	PackingResult PackingService::Pack(const PackingConfig& cfg)
	{
		PackingResult r;
		r.success = false;

		std::error_code ec;
		const auto rootDir = std::filesystem::path(ConfigManager::GetInstance().GetRootFolder());
		const auto outDir = std::filesystem::path(cfg.OutputDirectory);
		std::filesystem::create_directories(outDir, ec);
		if (ec && !std::filesystem::exists(outDir))
		{
			r.log.push_back("ERROR: 无法创建输出目录 " + cfg.OutputDirectory + " (" + ec.message() + ")");
			return r;
		}

		// 0. 拷贝目标 exe
		const auto srcExe = rootDir / cfg.TargetExe;
		if (!std::filesystem::exists(srcExe))
		{
			r.log.push_back("ERROR: 目标可执行体不存在：" + srcExe.string() + " — 请先构建再打包");
			return r;
		}

		const auto dstExe = outDir / cfg.TargetExe;
		std::filesystem::copy_file(srcExe, dstExe,
			cfg.Overwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::skip_existing,
			ec);
		if (ec && std::filesystem::exists(dstExe))
		{
			r.log.push_back("ERROR: 覆盖目标 exe 失败：" + dstExe.string() + " (" + ec.message() + ")");
			return r;
		}
		r.log.push_back("OK: 可执行体 " + cfg.TargetExe);

		// 1. Assets 资产（场景等）
		if (cfg.IncludeAssets)
		{
			const auto src = ConfigManager::GetInstance().GetRootFolder() / "Assets";
			if (!CopyRecursive(src, outDir / "Assets", cfg.Overwrite, r.log))
				return r;
		}

		// 2. Shaders（引擎必须加载）
		if (cfg.IncludeShaders)
		{
			const auto src = ConfigManager::GetInstance().GetRootFolder() / "Shaders";
			if (!CopyRecursive(src, outDir / "Shaders", cfg.Overwrite, r.log))
				return r;
		}

		// 3. Resources（字体等）
		if (cfg.IncludeResources)
		{
			const auto src = ConfigManager::GetInstance().GetRootFolder() / "Resources";
			if (!CopyRecursive(src, outDir / "Resources", cfg.Overwrite, r.log))
				return r;
		}

		// 4. 拷贝 MinGW 运行 DLL
		if (cfg.IncludeRuntimeDlls)
		{
			// MSYS2 ucrt64 下的核心 DLL，必须带着才能在干净系统跑
			const std::vector<std::string> dlls = {
				"libstdc++-6.dll",
				"libgcc_s_seh-1.dll",
				"libwinpthread-1.dll"
			};
			const char* mingwPath = "C:/msys64/ucrt64/bin";
			for (const auto& dll : dlls)
			{
				auto srcDll = std::filesystem::path(mingwPath) / dll;
				auto dstDll = outDir / dll;
				if (!std::filesystem::exists(srcDll))
				{
					r.log.push_back("WARN: MinGW DLL " + dll + " 未找到于 " + mingwPath + " — 跳过");
					continue;
				}
				std::filesystem::copy_file(srcDll, dstDll,
					cfg.Overwrite ? std::filesystem::copy_options::overwrite_existing : std::filesystem::copy_options::skip_existing,
					ec);
				if (ec && !std::filesystem::exists(dstDll))
				{
					r.log.push_back("ERROR: 拷贝 " + dll + " 失败 (" + ec.message() + ")");
					return r;
				}
				r.log.push_back("COPY: runtime DLL → " + dll);
			}
		}

		r.success = true;
		r.log.push_back("=== 打包完成 ===");
		return r;
	}

	void PackingService::OpenInExplorer(const std::string& dir)
	{
		// 使用 explorer.exe 打开目录
		ShellExecuteW(NULL, L"explore", std::filesystem::path(dir).wstring().c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
}