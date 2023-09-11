#pragma once

#include "xlpch.h"

#include "XLEngine/Log.h"
#include "XLEngine/Core.h"
#include "XLEngine/Events/Event.h"

namespace XLEngine
{
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "XLEngine",
			unsigned int width = 1200,
			unsigned int height = 720)
			:Title(title),Width(width),Height(height)
		{
		}
	};

	//基于桌面系统的窗口的界面
	class XLENGINE_API Window
	{
	public:
		using EventCallbackFn = std::function <void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};
}