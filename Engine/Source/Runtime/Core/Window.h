#pragma once

#include "xlpch.h"

#include "Runtime/Core/Base/Base.h"
#include "Runtime/Events/Event.h"

namespace XLEngine
{
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "XLEngine",
			uint32_t width = 1600,
			uint32_t height = 900)
			:Title(title),Width(width),Height(height)
		{
		}
	};

	//����Windowsϵͳ�Ĵ��ڵĽ���
	class XLENGINE_API Window
	{
	public:
		using EventCallbackFn = std::function <void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;
		[[nodiscard]] virtual uint32_t GetWidth() const = 0;
		[[nodiscard]] virtual uint32_t GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		// 为窗口设置标题栏/任务栏图标（加载 PNG，Bitmap 级零 GPU 依赖）
		virtual void SetTitleIcon(const std::string& pngPath) {}

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}