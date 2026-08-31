#include "xlpch.h"
#include "Runtime/Resource/AssetRegistry.h"
#include "Runtime/Resource/AssetDescriptor.h"
#include "Runtime/Resource/ConfigManager/ConfigManager.h"

#include <system_error>

namespace XLEngine
{
	void AssetRegistry::Initialize()
	{
		Clear();
		m_ContentRoot = ConfigManager::GetInstance().GetAssetsFolder();
		if (!std::filesystem::exists(m_ContentRoot))
			return;
		RegisterDirectory(m_ContentRoot, true);
	}

	void AssetRegistry::Clear()
	{
		m_ContentRoot.clear();
		m_PathToHandle.clear();
		m_HandleToMetadata.clear();
		m_RefCounts.clear();
	}

	AssetHandle AssetRegistry::CreateHandleFromRelativePath(const std::string& relativePath) const
	{
		return AssetHandle(AssetHandleHash(relativePath));
	}

	std::vector<AssetHandle> AssetRegistry::RegisterDirectory(const std::filesystem::path& directory, bool recursively)
	{
		std::vector<AssetHandle> handles;
		std::error_code ec;
		std::filesystem::directory_iterator end;
		std::filesystem::directory_iterator it(directory, ec);
		if (ec)
			return handles;

		for (; it != end; it.increment(ec))
		{
			if (ec)
				break;
			const auto& entry = *it;
			if (entry.is_directory(ec))
			{
				if (recursively)
				{
					auto sub = RegisterDirectory(entry.path(), recursively);
					handles.insert(handles.end(), sub.begin(), sub.end());
				}
			}
			else if (entry.is_regular_file(ec))
			{
				AssetHandle h = RegisterFile(entry.path());
				if (h.IsValid())
					handles.push_back(h);
			}
		}
		return handles;
	}

	AssetHandle AssetRegistry::RegisterFile(const std::filesystem::path& fullPath)
	{
		std::error_code ec;
		if (!std::filesystem::is_regular_file(fullPath, ec) || ec)
			return AssetHandle();

		// 跳过描述文件本身，避免把 ".xld" 也登记为资产
		const std::string ext = fullPath.extension().string();
		if (ext == ".xld")
			return AssetHandle();

		std::string relativePath = fullPath.string();
		if (!m_ContentRoot.empty())
		{
			auto rel = std::filesystem::relative(fullPath, m_ContentRoot, ec);
			if (!ec && !rel.empty())
				relativePath = rel.lexically_normal().generic_string();
		}

		// 已注册则直接返回既有句柄
		auto found = m_PathToHandle.find(relativePath);
		if (found != m_PathToHandle.end())
			return found->second;

		AssetHandle handle = CreateHandleFromRelativePath(relativePath);
		// 处理极小概率的句柄碰撞：已存在其它资产占用该句柄时，则加到相对路径末尾再哈希
		if (m_HandleToMetadata.find(handle) != m_HandleToMetadata.end())
			handle = CreateHandleFromRelativePath(relativePath + "-" + std::to_string(m_HandleToMetadata.size()));

		AssetMetadata metadata;
		metadata.Handle = handle;
		metadata.Type = AssetTypeFromExtension(ext);
		metadata.FilePath = fullPath;
		metadata.RelativePath = relativePath;
		metadata.Size = static_cast<std::uint64_t>(std::filesystem::file_size(fullPath, ec));

		m_HandleToMetadata[handle] = metadata;
		m_PathToHandle[relativePath] = handle;

		// 同步生成/核对统一资源描述文件（URDF）
		if (metadata.Type != AssetType::Other && metadata.Type != AssetType::None)
		{
			std::filesystem::path descriptorPath = fullPath;
			descriptorPath += ".xld";
			AssetMetadata existing;
			if (!std::filesystem::exists(descriptorPath) || !AssetDescriptor::Deserialize(existing, descriptorPath))
				AssetDescriptor::Serialize(metadata, descriptorPath);
		}

		return handle;
	}

	AssetHandle AssetRegistry::GetHandleFromPath(const std::string& relativePath) const
	{
		auto found = m_PathToHandle.find(relativePath);
		if (found != m_PathToHandle.end())
			return found->second;

		// 兜底：直接用哈希推导（对没扫到的路径也能给出稳定句柄）
		return CreateHandleFromRelativePath(relativePath);
	}
	AssetHandle AssetRegistry::GetHandleFromPath(const std::filesystem::path& fullPath) const
	{
		std::error_code ec;
		std::string relativePath = fullPath.string();
		if (!m_ContentRoot.empty())
		{
			auto rel = std::filesystem::relative(fullPath, m_ContentRoot, ec);
			if (!ec && !rel.empty())
				relativePath = rel.lexically_normal().generic_string();
		}
		return GetHandleFromPath(relativePath);
	}

	const AssetMetadata* AssetRegistry::GetMetadata(AssetHandle handle) const
	{
		auto found = m_HandleToMetadata.find(handle);
		if (found != m_HandleToMetadata.end())
			return &found->second;
		return nullptr;
	}

	AssetType AssetRegistry::GetAssetType(AssetHandle handle) const
	{
		const AssetMetadata* md = GetMetadata(handle);
		return md ? md->Type : AssetType::None;
	}
	AssetType AssetRegistry::GetAssetType(const std::filesystem::path& fullPath) const
	{
		return AssetTypeFromExtension(fullPath.extension().string());
	}

	bool AssetRegistry::IsRegistered(AssetHandle handle) const
	{
		return m_HandleToMetadata.find(handle) != m_HandleToMetadata.end();
	}

	void AssetRegistry::Acquire(AssetHandle handle)
	{
		if (!IsRegistered(handle))
			return;
		auto it = m_RefCounts.find(handle);
		if (it == m_RefCounts.end())
			m_RefCounts[handle] = 1;
		else
			++it->second;
	}
	void AssetRegistry::Release(AssetHandle handle)
	{
		auto it = m_RefCounts.find(handle);
		if (it == m_RefCounts.end())
			return;
		if (it->second <= 1)
			m_RefCounts.erase(it);
		else
			--it->second;
	}
	std::uint32_t AssetRegistry::GetRefCount(AssetHandle handle) const
	{
		auto it = m_RefCounts.find(handle);
		return it == m_RefCounts.end() ? 0u : it->second;
	}
}