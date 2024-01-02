#include "xlpch.h"
#include "Runtime/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Runtime/Core/Application.h"

namespace XLEngine
{
	// ¸¨Öúº¯Êý£º½«Õ­×Ö·û×ª»»Îª¿í×Ö·û
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
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// Ê¹ÓÃ¸¨Öúº¯Êý½øÐÐ×ª»»
		std::wstring wideFilter = ConvertToWideString(filter);
		ofn.lpstrFilter = reinterpret_cast<LPCSTR>(wideFilter.c_str()); // ½«¿í×Ö·û×Ö·û´®Ç¿ÖÆ×ª»»ÎªÕ­×Ö·û×Ö·û´®
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
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// Ê¹ÓÃ¸¨Öúº¯Êý½øÐÐ×ª»»
		std::wstring wideFilter = ConvertToWideString(filter);
		ofn.lpstrFilter = reinterpret_cast<LPCSTR>(wideFilter.c_str()); // ½«¿í×Ö·û×Ö·û´®Ç¿ÖÆ×ª»»ÎªÕ­×Ö·û×Ö·û´®
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}
}
