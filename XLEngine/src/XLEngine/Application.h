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
	private:
		bool m_Running = true;
	};
	
	//�ڿͻ��˶���
	Application* CreateApplication();
}
