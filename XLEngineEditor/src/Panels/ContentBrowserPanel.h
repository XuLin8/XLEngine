#pragma once

#include <filesystem>

namespace XLEngine
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		void OnImGuiRender(bool* pOpen);
	private:
		std::filesystem::path m_CurrentDirectory;
	};
}