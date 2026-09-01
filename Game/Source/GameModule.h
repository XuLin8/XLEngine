#pragma once
#include "Runtime/EcsFramework/Level/Level.h"

namespace XLEngine
{
	// Game 模块装配入口：宿主（Editor/可执行体）通过该工厂把玩法系统注入关卡。
	// 引擎（XLEngineRuntime / XLEngineEditor）只依赖这个头面，不感知任何具体玩法类型，
	// 从而保证依赖方向严格单向：Game → Engine。
	namespace GameModule
	{
		// 装配玩法系统到关卡（编辑态持久关卡、运行态运行关卡均调用）。
		void RegisterGameplay(Ref<Level> level);

		// HUD 进度快照：无对应玩法系统时 Active=false，其余字段归零。
		struct HudInfo
		{
			int  MotesCollected = 0;
			int  MotesTotal     = 0;
			int  BeaconsLit     = 0;
			bool Dawn           = false;
			bool Active         = false;
		};
		HudInfo QueryHud(Ref<Level> level);
	}
}