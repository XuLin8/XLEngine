#include "xlpch.h"
#include "Runtime/Core/AppFramework/Application.h"

#ifdef XL_PLATFORM_WINDOWS

namespace XLEngine
{
	// To be defined in CLIENT
	extern void MyAppInitialize(Application& app);
}
int main(int argc, char** argv)
{
	XL_PROFILE_BEGIN_SESSION("MyAppInitialize", "XLEngineProfile-MyAppInitialize.json");
	XLEngine::MyAppInitialize(XLEngine::Application::GetInstance());
	XL_PROFILE_END_SESSION();

	XL_PROFILE_BEGIN_SESSION("Run", "XLEngineProfile-Run.json");
	XLEngine::Application::GetInstance().Run();
	XL_PROFILE_END_SESSION();

	XL_PROFILE_BEGIN_SESSION("Clean", "XLEngineProfile-Clean.json");
	XLEngine::Application::GetInstance().Clean();
	XL_PROFILE_END_SESSION();
}

#endif
