#include "xlpch.h"
#include "Runtime/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Runtime/Core/AppFramework/Application.h"

namespace XLEngine
{
	// 辅助函数：将窄字符转换为宽字符
	static std::wstring ConvertToWideString(const char* narrowString)
	{
		int length = MultiByteToWideChar(CP_ACP, 0, narrowString, -1, nullptr, 0);
		std::wstring wideString;
		if (length > 0)
		{
			wideString.resize(length);
			MultiByteToWideChar(CP_ACP, 0, narrowString, -1, &wideString[0], length);
		}
		return wideString;
	}

	std::string FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetInstance().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// 使用辅助函数进行转换
		std::wstring wideFilter = ConvertToWideString(filter);
		ofn.lpstrFilter = reinterpret_cast<LPCSTR>(wideFilter.c_str()); // 将宽字符字符串强制转换为窄字符字符串
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
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetInstance().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// 使用辅助函数进行转换
		std::wstring wideFilter = ConvertToWideString(filter);
		ofn.lpstrFilter = reinterpret_cast<LPCSTR>(wideFilter.c_str()); // 将宽字符字符串强制转换为窄字符字符串
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}
}
