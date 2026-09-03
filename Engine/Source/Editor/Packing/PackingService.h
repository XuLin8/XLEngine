#pragma once

#include <string>
#include <vector>

namespace XLEngine
{
	// 打包配置：要打包的构建产物、输出目录，以及复选的"内容"。
	struct PackingConfig
	{
		std::string TargetExe;         // 目标可执行体文件名，如 "XLEngineGameApp.exe"
		std::string OutputDirectory;   // 输出目录（不存在将自动创建）
		bool IncludeAssets    = true;  // 复制 Assets/（场景/纹理/模型等内容资产）
		bool IncludeShaders   = true;  // 复制 Shaders/（着色器）
		bool IncludeResources = true;  // 复制 Resources/（图标/字体等编辑器资源）
		bool IncludeRuntimeDlls = true;// 复制 MinGW 分发所需 DLL（libstdc++/libgcc/libwinpthread）
		bool Overwrite        = false; // 已存在文件是否覆盖
	};

	struct PackingResult
	{
		bool success = false;             // 是否成功
		std::vector<std::string> log;     // 逐条操作日志（供 UI 展示）
	};

	// 打包服务：把构建目录（ConfigManager 根目录）中的目标可执行体 + 复选内容
	// 拷贝到输出目录，生成可分发的游戏包体。UI 无关，可被任意宿主复用。
	class PackingService
	{
	public:
		// 当前构建目录下可打包的可执行体候选（存在才列出）
		static std::vector<std::string> DiscoverExeCandidates();
		static bool HasExe(const std::string& name);

		// 执行打包：目标 exe 必选，内容按配置复选。
		static PackingResult Pack(const PackingConfig& cfg);

		// 在资源管理器中打开目录
		static void OpenInExplorer(const std::string& dir);
	};
}