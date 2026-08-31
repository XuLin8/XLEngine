#pragma once

#include "AssetHandle.h"
#include "AssetType.h"

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

namespace XLEngine
{
	// 资产加载状态
	enum class AssetStatus : std::uint8_t
	{
		Unloaded = 0,   // 仅登记路径/元数据，尚未加载到内存
		Loaded,         // 已加载
		Error           // 加载失败
	};

	// 单个资产的元数据。AssetRegistry 以句柄为键存取。
	struct AssetMetadata
	{
		AssetHandle Handle;                   // 资产唯一句柄
		AssetType   Type = AssetType::None;   // 资产类型
		std::filesystem::path FilePath;       // 完整文件系统路径
		std::string RelativePath;             // 相对 ContentRoot 的路径（也用于句柄哈希）
		std::uint64_t Size = 0;               // 文件字节数
		AssetStatus Status = AssetStatus::Unloaded;

		// 依赖的其它资源句柄（预留：后续用于依赖加载/卸载与 GC）
		std::vector<AssetHandle> Dependencies;

		[[nodiscard]] bool IsValid() const noexcept { return Handle.IsValid() && Type != AssetType::None; }
	};
}