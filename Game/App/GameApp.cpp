#include "XLEngine.h"

#include "GameLayer.h"

namespace XLEngine
{
	// 独立游戏可执行体入口（打包 exe）：复用 Launch.cpp 提供的 main，
	// 仅装配引擎 Runtime + Game 玩法模块，并直接进入运行时游玩关卡。
	void MyAppInitialize(Application& app)
	{
		app.Init("Lumen Isle");

		app.PushLayer(new GameLayer());
	}
}