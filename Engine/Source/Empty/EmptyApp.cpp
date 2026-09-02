#include "XLEngine.h"

#include "EmptyLayer.h"

namespace XLEngine
{
	// 空引擎入口（E3 验收）：仅装配引擎 Runtime，不链接任何 Game 玩法模块。
	// 复用 Launch.cpp 提供的 main：此处只需初始化 Application 并推入一个空关卡层。
	void MyAppInitialize(Application& app)
	{
		app.Init("XLEngine Empty");

		app.PushLayer(new EmptyLayer());
	}
}