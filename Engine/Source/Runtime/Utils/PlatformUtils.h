#pragma once
#include <string>

namespace XLEngine
{
	class FileDialogs
	{
	public:
		// These return empty strings if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);

		// 由应用框架层注入原生窗口句柄（作为原生文件对话框的父窗口），
		// 避免平台层反向依赖 Application 单例。
		static void SetOwnerWindow(void* nativeWindow);
	private:
		static void* s_OwnerWindow;
	};
}