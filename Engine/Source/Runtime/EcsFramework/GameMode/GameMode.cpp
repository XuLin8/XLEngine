#include "xlpch.h"
#include "Runtime/EcsFramework/GameMode/GameMode.h"
#include "Runtime/EcsFramework/Level/Level.h"

namespace XLEngine
{
	void DefaultGameMode::BeginPlay(Level* level)
	{
		if (level)
			level->OnRuntimeStart();
	}

	void DefaultGameMode::EndPlay(Level* level)
	{
		if (level)
			level->OnRuntimeStop();
	}

	void DefaultGameMode::Tick(Timestep ts, Level* level, EditorCamera* camera)
	{
		if (!level)
			return;
		// 运行模式：把冻结的渲染/基准相机注入运行关卡，再驱动其玩法系统
		if (camera)
			level->SetRuntimeCamera(camera);
		level->OnUpdateRuntime(ts);
	}
}