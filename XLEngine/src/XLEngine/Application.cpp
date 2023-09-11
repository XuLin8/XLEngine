#include "Application.h"

namespace XLEngine
{
	Application::Application()
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		WindowResizeEvent e(1200, 700);
		XL_TRACE(e);

		while (true);
	}
}