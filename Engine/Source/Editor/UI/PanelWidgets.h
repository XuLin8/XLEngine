#pragma once

#include "imgui.h"
#include "Runtime/Utils/PlatformUtils.h"

#include <cstring>
#include <string>

// 可复用的编辑器 UI 小部件（header-only）。打包面板、路径设置等通用组件集中于此，
// 其它面板可复用，避免各自重复实现"标签 + 输入框 + 浏览"等常见交互。
namespace XLEngine::UI
{
	// 通用"标签 + 文本输入 + 浏览…"行：label 位于左侧，右侧为可编辑路径与「…」按钮。
	// 改变时会写入 value 并返回 true（含点「…」用文件夹选择器选中的情况）。
	// 适用于输出目录、资源目录等任意路径录入场景。
	inline bool PathField(const char* label, std::string& value)
	{
		bool changed = false;

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::SameLine();

		ImGui::PushID(label);

		// 输入框占满除右侧「…」按钮外的宽度
		float textWidth = ImGui::GetContentRegionAvail().x - 34.0f;
		ImGui::SetNextItemWidth(textWidth);
		char buf[1024];
		const size_t n = value.size() < sizeof(buf) - 1 ? value.size() : sizeof(buf) - 1;
		memcpy(buf, value.c_str(), n);
		buf[n] = '\0';
		if (ImGui::InputText("##path", buf, sizeof(buf)))
		{
			value = buf;
			changed = true;
		}

		ImGui::SameLine();
		if (ImGui::Button("..."))
		{
			std::string picked = FileDialogs::PickFolder();
			if (!picked.empty())
			{
				value = picked;
				changed = true;
			}
		}

		ImGui::PopID();
		return changed;
	}
}