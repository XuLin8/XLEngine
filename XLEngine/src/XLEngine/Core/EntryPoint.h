#pragma once

#ifdef XL_PLATFORM_WINDOWS

extern XLEngine::Application* XLEngine::CreateApplication();

int main(int argc, int argv)
{
	XLEngine::Log::Init();
	XL_CORE_WARN("Initialized Log!");
	
	XL_PROFILE_BEGIN_SESSION("Startup", "XLEngineProfile-Startup.json");
	auto app = XLEngine::CreateApplication();
	XL_PROFILE_END_SESSION();

	XL_PROFILE_BEGIN_SESSION("Runtime", "XLEngineProfile-Runtime.json");
	app->Run();
	XL_PROFILE_END_SESSION();

	XL_PROFILE_BEGIN_SESSION("Shutdown", "XLEngineProfile-Shutdown.json");
	delete app;
	XL_PROFILE_END_SESSION();
}

#endif
