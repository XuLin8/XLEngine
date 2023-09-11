#pragma once
#include "Core.h"
#include "Platform/Windows/Window.h"

namespace XLEngine
{
	class XLENGINE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};
	
	//在客户端定义
	Application* CreateApplication();
}
