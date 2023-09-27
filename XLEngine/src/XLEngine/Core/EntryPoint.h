#pragma once

#ifdef XL_PLATFORM_WINDOWS

extern XLEngine::Application* XLEngine::CreateApplication();

int main(int argc, int argv)
{
	XLEngine::Log::Init();
	XL_CORE_WARN("Initialized Log!");
	int a = 5;
	XL_INFO("Hello! Var = {0}", a);

	auto app = XLEngine::CreateApplication();
	app->Run();
	delete app;
}

#endif
