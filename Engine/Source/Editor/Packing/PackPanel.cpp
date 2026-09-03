#include "xlpch.h"
#include "PackPanel.h"

#include "Runtime/Resource/ConfigManager/ConfigManager.h"
#include "Runtime/Utils/PlatformUtils.h"
#include "../UI/PanelWidgets.h"

#include <filesystem>
#include <imgui.h>

namespace XLEngine
{
	void PackPanel::Reset()
	{
		// 默认输出目录：构建目录的同级 BuildPack 文件夹（构建目录 = ConfigManager 根目录）
		if (m_Config.OutputDirectory.empty())
		{
			const auto root = std::filesystem::path(ConfigManager::GetInstance().GetRootFolder());
			m_Config.OutputDirectory = (root.parent_path() / "BuildPack").string();
		}
		// 默认目标优先指向游戏可执行体
		if (m_Config.TargetExe.empty())
		{
			const auto exes = PackingService::DiscoverExeCandidates();
			for (const auto& e : exes)
			{
				if (e.rfind("GameApp", 0) == 0 || e.find("GameApp") != std::string::npos)
				{
					m_Config.TargetExe = e;
					break;
				}
			}
			if (m_Config.TargetExe.empty() && !exes.empty())
				m_Config.TargetExe = exes.front();
		}
		m_Result = PackingResult{};
		m_bHasPacked = false;
	}

	void PackPanel::OnImGuiRender(bool* pOpen)
	{
		ImGui::Begin("打包 Build Package", pOpen);

		ImGui::TextWrapped("将构建产物整合为可分发的游戏包体（.exe + 内容资源）。");

		// ---- 目标可执行体 ----
		ImGui::Separator();
		ImGui::TextUnformatted("目标可执行体 (Target .exe)");
		const auto exes = PackingService::DiscoverExeCandidates();
		if (exes.empty())
		{
			ImGui::TextDisabled("（未发现已构建 .exe，请先在 VSCode 运行构建任务）");
		}
		else
		{
			if (ImGui::BeginCombo("##target_exe", m_Config.TargetExe.c_str()))
			{
				for (const auto& name : exes)
				{
					bool selected = (name == m_Config.TargetExe);
					if (ImGui::Selectable(name.c_str(), selected))
						m_Config.TargetExe = name;
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// ---- 输出路径（复用可复用组件 UI::PathField）----
		UI::PathField("输出目录", m_Config.OutputDirectory);

		// ---- 打包内容（复选）----
		ImGui::Separator();
		ImGui::TextUnformatted("打包内容 (Content)");
		ImGui::Checkbox("场景与内容资产  Assets/", &m_Config.IncludeAssets);
		ImGui::Checkbox("着色器           Shaders/", &m_Config.IncludeShaders);
		ImGui::Checkbox("编辑器资源       Resources/", &m_Config.IncludeResources);
		ImGui::Checkbox("MinGW 运行库 DLL (libstdc++/gcc/winpthread)", &m_Config.IncludeRuntimeDlls);

		ImGui::Separator();
		ImGui::Checkbox("覆盖已存在文件", &m_Config.Overwrite);

		// ---- 操作按钮 ----
		ImGui::Separator();
		if (ImGui::Button("开始打包 (Pack)"))
		{
			m_Result = PackingService::Pack(m_Config);
			m_bHasPacked = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("在资源管理器中打开", ImVec2{ 0, 0 }) && !m_Config.OutputDirectory.empty())
		{
			PackingService::OpenInExplorer(m_Config.OutputDirectory);
		}
		ImGui::SameLine();
		if (ImGui::Button("关闭 (Close)"))
		{
			*pOpen = false;
		}

		// ---- 结果日志 ----
		if (m_bHasPacked)
		{
			ImGui::Separator();
			ImGui::TextUnformatted(m_Result.success ? "[成功] 打包完成" : "[失败] 打包未成功，查看日志");
		}

		ImGui::Separator();
		ImGui::BeginChild("##packlog", ImVec2(0.0f, ImGui::GetContentRegionAvail().y));
		for (const auto& line : m_Result.log)
			ImGui::TextUnformatted(line.c_str());
		ImGui::EndChild();

		ImGui::End();
	}
}