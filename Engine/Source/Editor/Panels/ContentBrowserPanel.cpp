#include "ContentBrowserPanel.h"
#include "Runtime/Resource/ConfigManager/ConfigManager.h"
#include "Runtime/Resource/AssetManager/AssetManager.h"
#include "Runtime/Resource/AssetRegistry.h"
#include "Runtime/Resource/AssetType.h"

#include <imgui/imgui.h>

#include <filesystem>
#include <system_error>

namespace XLEngine
{
	ContentBrowserPanel::ContentBrowserPanel()
		:m_CurrentDirectory(ConfigManager::GetInstance().GetAssetsFolder())
	{
		m_DirectoryIcon = Texture2D::Create(AssetManager::GetInstance().GetFullPath("Resources/Icons/ContentBrowser/DirectoryIcon.png").string());
		m_FileIcon = Texture2D::Create(AssetManager::GetInstance().GetFullPath("Resources/Icons/ContentBrowser/FileIcon.png").string());
	}

	Ref<Texture2D> ContentBrowserPanel::GetThumbnail(AssetHandle handle, const std::filesystem::path& fullPath)
	{
		auto found = m_Thumbnails.find(handle.Get());
		if (found != m_Thumbnails.end())
			return found->second;
		// 纹理之外的资产用通用文件图标
		if (AssetTypeFromExtension(fullPath.extension().string()) != AssetType::Texture)
			return m_FileIcon;

		Ref<Texture2D> tex = Texture2D::Create(fullPath);
		m_Thumbnails[handle.Get()] = tex;
		return tex;
	}

	void ContentBrowserPanel::OnAssetClicked(AssetHandle handle, const AssetMetadata& metadata)
	{
		m_SelectedAsset = handle;
		m_SelectedType = metadata.Type;
		m_SelectedRelativePath = metadata.RelativePath;
		m_PreviewTexture = (metadata.Type == AssetType::Texture) ? GetThumbnail(handle, metadata.FilePath) : nullptr;
	}

	void ContentBrowserPanel::OnImGuiRender(bool* pOpen)
	{
		if (!ImGui::Begin("Content Browser", pOpen))
		{
			ImGui::End();
			return;
		}

		AssetRegistry& registry = AssetRegistry::GetInstance();

		if (m_CurrentDirectory != ConfigManager::GetInstance().GetAssetsFolder())
		{
			if (ImGui::Button("<<"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
				m_SelectedAsset = AssetHandle();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		std::error_code ec;
		std::filesystem::directory_iterator end;
		std::filesystem::directory_iterator it(m_CurrentDirectory, ec);
		if (ec)
		{
			ImGui::End();
			return;
		}

		for (; it != end; it.increment(ec))
		{
			if (ec)
				break;
			const auto& entry = *it;
			const auto& path = entry.path();
			std::string filenameString = path.filename().string();

			ImGui::PushID(filenameString.c_str());

			bool isDirectory = entry.is_directory(ec);
			Ref<Texture2D> icon;
			std::string typeLabel;
			AssetHandle itemHandle;
			const AssetMetadata* itemMd = nullptr;

			if (isDirectory)
			{
				icon = m_DirectoryIcon;
			}
			else
			{
				// 从注册表获取资产元数据（未登记则先登记）
				itemHandle = registry.RegisterFile(path);
				itemMd = registry.GetMetadata(itemHandle);
				if (itemMd)
				{
					typeLabel = std::string(AssetTypeToString(itemMd->Type));
					icon = GetThumbnail(itemHandle, itemMd->FilePath);
				}
				else
				{
					icon = m_FileIcon;
				}
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::PopStyleColor();

			bool hovered = ImGui::IsItemHovered();
			// 单击选中（单选按钮）
			if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				if (itemMd)
					OnAssetClicked(itemHandle, *itemMd);
				else
					m_SelectedAsset = AssetHandle();
			}
			// 双击：目录进入 / 资产回调打开
			if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (isDirectory)
				{
					m_CurrentDirectory /= path.filename();
					m_SelectedAsset = AssetHandle();
				}
				else if (m_OpenAssetCallback)
				{
					m_OpenAssetCallback(path);
				}
			}

			ImGui::TextWrapped("%s", filenameString.c_str());
			if (!typeLabel.empty())
				ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.0f, 1.0f), "%s", typeLabel.c_str());

			ImGui::NextColumn();
			ImGui::PopID();
		}
		ImGui::Columns(1);

		ImGui::Separator();

		// ---- 资产预览 ----
		if (m_SelectedAsset.IsValid())
		{
			ImGui::Text("Asset Preview");
			ImGui::Text("Type: %s", std::string(AssetTypeToString(m_SelectedType)).c_str());
			ImGui::Text("Path: %s", m_SelectedRelativePath.c_str());
			ImGui::Text("RefCount: %u", registry.GetRefCount(m_SelectedAsset));
			if (m_PreviewTexture)
			{
				ImVec2 avail = ImGui::GetContentRegionAvail();
				float ratio = (float)m_PreviewTexture->GetHeight() / (float)std::max(1u, m_PreviewTexture->GetWidth());
				float w = std::min(avail.x, 256.0f);
				ImGui::Image((ImTextureID)m_PreviewTexture->GetRendererID(), ImVec2(w, w * ratio));
			}
		}

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);

		ImGui::End();
	}
}