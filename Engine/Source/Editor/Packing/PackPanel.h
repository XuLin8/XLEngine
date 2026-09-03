#pragma once

#include "PackingService.h"

#include <string>
#include <vector>

namespace XLEngine
{
	// 打包面板：编辑器「Build → 打包…」弹出的窗口。
	// 复用 PackingService（后端）与 UI 小部件（路径录入），满足"组件复用"：
	// 内容复选、目标可执行体、输出路径、覆盖开关、开始/打开/关闭、结果日志区。
	class PackPanel
	{
	public:
		void OnImGuiRender(bool* pOpen);

		// 打开面板时（在菜单点击处）调用，用于恢复上次会话并预填默认输出目录
		void Reset();

	private:
		PackingConfig m_Config;
		PackingResult m_Result;
		bool m_bHasPacked = false;
	};
}