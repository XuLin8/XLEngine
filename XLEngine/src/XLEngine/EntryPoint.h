#pragma once

#ifdef XL_PLATFORM_WINDOWS

extern XLEngine::Application* XLEngine::CreateApplication();

int main(int argc, int argv)
{
	printf("XLEngine\n");
	auto app = XLEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif
