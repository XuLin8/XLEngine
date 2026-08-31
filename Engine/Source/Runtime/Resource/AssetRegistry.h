#pragma once

#include "Runtime/Core/Base/PublicSingleton.h"
#include "Runtime/Resource/AssetHandle.h"
#include "Runtime/Resource/AssetMetadata.h"

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace XLEngine
{
	// AssetRegistry：统一资产注册表。
	//  路径(相对 ContentRoot)  ->  句柄  ->  元数据 + 引用计数
	// 负责扫描资产目录、登记资产、提供常量查询与引用计数管理。
	// 语义向 UE 的 Object 注册表 / 资产注册表靠拢（元数据层，不负责载入具体载荷）。
	class AssetRegistry : public PublicSingleton<AssetRegistry>
	{
	public:
		void Initialize();
		void Clear();

		// 扫描目录并注册其中已知类型的资产（可选递归），返回注册的句柄列表。
		std::vector<AssetHandle> RegisterDirectory(const std::filesystem::path& directory, bool recursively = true);
		// 注册单个文件，返回其句柄（重复注册返回既有句柄）。
		AssetHandle RegisterFile(const std::filesystem::path& fullPath);

		// ---- 查询 ----

		// 相对 ContentRoot 的路径 -> 句柄；未注册返回无效句柄
		AssetHandle GetHandleFromPath(const std::string& relativePath) const;
		// 完整路径 -> 句柄（内部会转成相对路径）
		AssetHandle GetHandleFromPath(const std::filesystem::path& fullPath) const;
		const AssetMetadata* GetMetadata(AssetHandle handle) const;
		AssetType GetAssetType(AssetHandle handle) const;
		AssetType GetAssetType(const std::filesystem::path& fullPath) const;
		bool IsRegistered(AssetHandle handle) const;

		// ---- 引用计数：Acquire/Release 成对使用，0 表示无引用（资源可被卸载） ----
		void Acquire(AssetHandle handle);
		void Release(AssetHandle handle);
		std::uint32_t GetRefCount(AssetHandle handle) const;

		// ---- 迭代 ----
		template<typename Fn>
		void ForEach(Fn&& fn) const
		{
			for (const auto& kv : m_HandleToMetadata)
				fn(kv.second);
		}
		[[nodiscard]] std::size_t GetCount() const { return m_HandleToMetadata.size(); }

		[[nodiscard]] const std::filesystem::path& GetContentRoot() const { return m_ContentRoot; }
		void SetContentRoot(const std::filesystem::path& root) { m_ContentRoot = root; }

	private:
		AssetHandle CreateHandleFromRelativePath(const std::string& relativePath) const;

		std::filesystem::path m_ContentRoot;
		std::unordered_map<std::string, AssetHandle> m_PathToHandle;                  // 相对路径 -> 句柄
		std::unordered_map<AssetHandle, AssetMetadata, AssetHandleHashFn> m_HandleToMetadata; // 句柄 -> 元数据
		std::unordered_map<AssetHandle, std::uint32_t, AssetHandleHashFn> m_RefCounts;       // 句柄 -> 引用计数
	};
}