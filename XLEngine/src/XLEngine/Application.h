#pragma once
#include "Core.h"
#include "Window.h"
#include "Events/ApplicationEvent.h"

namespace XLEngine
{
	class XLENGINE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};
	
	//在客户端定义
	Application* CreateApplication();
}
