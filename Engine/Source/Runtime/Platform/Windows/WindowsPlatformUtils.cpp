#include "xlpch.h"
#include "Runtime/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace XLEngine
{
	void* FileDialogs::s_OwnerWindow = nullptr;

	void FileDialogs::SetOwnerWindow(void* nativeWindow)
	{
		s_OwnerWindow = nativeWindow;
	}

	std::string FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)s_OwnerWindow);
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// GetOpenFileNameA 为 ANSI 接口：filter 是含 \0 分隔的多段窄串
		// （如 "XLEngine Scene (*.xl)\0*.xl\0\0"），必须原样直传，不可转宽再强转。
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)s_OwnerWindow);
		ofn.nMaxFile = sizeof(szFile);
		// 同 OpenFile：ANSI 接口，filter 多段窄串原样直传
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}
}
