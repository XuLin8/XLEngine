#pragma once
#include "Core.h"

namespace XLEngine
{
	class XLENGINE_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};
	
	//�ڿͻ��˶���
	Application* CreateApplication();
}
