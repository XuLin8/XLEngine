#pragma once

#include "Runtime/Renderer/Texture.h"
#include "Runtime/Resource/AssetMetadata.h"

#include <filesystem>
#include <functional>
#include <unordered_map>

namespace XLEngine
{
	// 资源浏览器：由 AssetRegistry 驱动，列出资产目录下的资源，
	// 显示类型标签、为纹理生成缩略图，选中后在下方面板预览；
	// 双击 .xl 场景资源时通过回调交给 EditorLayer 加载。
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		void OnImGuiRender(bool* pOpen);

		// 双击资产（如场景）时回调，由 EditorLayer 注入，用于加载/打开资产
		void SetOpenAssetCallback(const std::function<void(const std::filesystem::path&)>& cb) { m_OpenAssetCallback = cb; }

	private:
		Ref<Texture2D> GetThumbnail(AssetHandle handle, const std::filesystem::path& fullPath);
		void OnAssetClicked(AssetHandle handle, const AssetMetadata& metadata);

		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;

		std::function<void(const std::filesystem::path&)> m_OpenAssetCallback;

		AssetHandle m_SelectedAsset;
		AssetType m_SelectedType = AssetType::None;
		std::string m_SelectedRelativePath;
		Ref<Texture2D> m_PreviewTexture;
		std::unordered_map<std::uint64_t, Ref<Texture2D>> m_Thumbnails;
	};
}