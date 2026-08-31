#pragma once

#include "Runtime/Core/Timestep.h"
#include "Runtime/Renderer/EditorCamera.h"

namespace XLEngine
{
	class Level;

	// GameMode：UE GameMode 的轻量对应，承载一场运行时(Play)会话的规则。
	// 它拥有会话生命周期钩子（BeginPlay/EndPlay/Tick），与关卡数据解耦，
	// 便于后续替换成其它玩法规则而不触碰 Level/World。
	class GameMode
	{
	public:
		GameMode() = default;
		virtual ~GameMode() = default;

		GameMode(const GameMode&) = delete;
		GameMode& operator=(const GameMode&) = delete;

		// 进入运行时：level 为 World 提供的运行关卡副本（非空）
		virtual void BeginPlay(Level* level) {}
		virtual void EndPlay(Level* level) {}
		// 每帧驱动：camera 为运行模式下冻结的渲染/基准相机，可为空
		virtual void Tick(Timestep ts, Level* level, EditorCamera* camera) {}
	};

	// 默认 GameMode：把生命周期与更新转发给 Level 内置系统（即 GameSystem 上的 M2 玩法）。
	class DefaultGameMode final : public GameMode
	{
	public:
		void BeginPlay(Level* level) override;
		void EndPlay(Level* level) override;
		void Tick(Timestep ts, Level* level, EditorCamera* camera) override;
	};
}