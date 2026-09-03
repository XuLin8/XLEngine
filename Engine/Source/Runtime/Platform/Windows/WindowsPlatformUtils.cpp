#include "xlpch.h"
#include "Runtime/Utils/PlatformUtils.h"

#include <commdlg.h>
#include <shlobj.h>
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

	std::string FileDialogs::PickFolder()
	{
		// 用 IFileOpenDialog 的文件夹模式 (FOS_PICKFOLDERS) 选择一个目录（COM）。
		HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
		if (hr != S_OK && hr != S_FALSE) // S_FALSE = 已在当前线程初始化，同样可用
			return std::string();

		std::string result;
		IFileOpenDialog* dialog = nullptr;
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&dialog))))
		{
			DWORD opts;
			dialog->GetOptions(&opts);
			dialog->SetOptions(opts | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);
			if (dialog->Show(glfwGetWin32Window((GLFWwindow*)s_OwnerWindow)) == S_OK)
			{
				IShellItem* item = nullptr;
				if (dialog->GetResult(&item) == S_OK)
				{
					PWSTR path = nullptr;
					if (item->GetDisplayName(SIGDN_FILESYSPATH, &path) == S_OK && path)
					{
						// 宽路径转 UTF-8（引擎内部统一窄串 std::string）
						int len = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
						result.assign(len - 1, '\0');
						WideCharToMultiByte(CP_UTF8, 0, path, -1, &result[0], len, nullptr, nullptr);
						CoTaskMemFree(path);
					}
					item->Release();
				}
			}
			dialog->Release();
		}
		CoUninitialize();
		return result;
	}
}
