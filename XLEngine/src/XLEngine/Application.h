#pragma once
#include "Core.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace XLEngine
{
	class XLENGINE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};
	
	//在客户端定义
	Application* CreateApplication();
}
