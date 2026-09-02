#include "xlpch.h"
#include "Runtime/Core/AppFramework/Application.h"
#include "Runtime/Core/Debug/CrashHandler.h"

#ifdef XL_PLATFORM_WINDOWS

namespace XLEngine
{
	// To be defined in CLIENT
	extern void MyAppInitialize(Application& app);
}
int main(int argc, char** argv)
{
	// 最先安装崩溃处理器：任何未处理异常/信号都会在终止前把故障地址与调用栈写入日志
	XLEngine::CrashHandler::Install();

	XL_PROFILE_BEGIN_SESSION("MyAppInitialize", "XLEngineProfile-MyAppInitialize.json");
	// 透传命令行参数（如 --play 启动即进入运行时模式），供工具层/玩法装配查询
	XLEngine::Application& app = XLEngine::Application::GetInstance();
	app.m_Args.assign(argv + 1, argv + argc);
	XLEngine::MyAppInitialize(app);
	XL_PROFILE_END_SESSION();

	XL_PROFILE_BEGIN_SESSION("Run", "XLEngineProfile-Run.json");
	app.Run();
	XL_PROFILE_END_SESSION();

	XL_PROFILE_BEGIN_SESSION("Clean", "XLEngineProfile-Clean.json");
	app.Clean();
	XL_PROFILE_END_SESSION();
}

#endif
