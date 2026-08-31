#pragma once

#include "Runtime/Core/Base/Base.h"
#include "Runtime/Core/Timestep.h"
#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/EcsFramework/GameMode/GameMode.h"
#include "Runtime/Renderer/EditorCamera.h"

#include <cstdint>

namespace XLEngine
{
	class GameSystem;

	// World：持有"持久关卡"(编辑源)与"运行关卡"(Play 时的副本)，并在编辑/运行时态间切换，
	// 同时调度 GameMode 的会话生命周期。对应 UE 的 World + 当前 Level + GameMode，
	// 让工具的编辑态与引擎的运行态在数据结构上彻底解耦。
	class World
	{
	public:
		enum class EState : std::uint8_t
		{
			Edit = 0,     // 编辑态：操作持久关卡 m_PersistentLevel
			Runtime       // 运行态：驱动运行关卡 m_RuntimeLevel
		};

		World() = default;
		~World() = default;
		World(const World&) = delete;
		World& operator=(const World&) = delete;

		// 设定被编辑的持久关卡（即当前 .xl 场景）
		void SetPersistentLevel(Ref<Level> level);

		// 编辑 -> 运行时：拷贝持久关卡为运行关卡，并 BeginPlay
		void Play();
		// 运行时 -> 编辑：EndPlay 并释放运行关卡，回到持久关卡
		void Stop();
		// 每帧驱动：runtime 为 true 走玩法(GameMode)，否则走编辑更新
		void Update(Timestep ts, EditorCamera* camera, bool runtime);

		void OnViewportResize(std::uint32_t width, std::uint32_t height);

		[[nodiscard]] Ref<Level> GetPersistentLevel() const { return m_PersistentLevel; }
		// 当前生效关卡：运行态返回运行关卡，编辑态返回持久关卡
		[[nodiscard]] Ref<Level> GetActiveLevel() const;
		[[nodiscard]] EState GetState() const { return m_State; }

		void SetGameMode(Ref<GameMode> gameMode) { m_GameMode = gameMode ? gameMode : CreateRef<DefaultGameMode>(); }
		[[nodiscard]] Ref<GameMode> GetGameMode() const { return m_GameMode; }

		// 透传给运行关卡，供 HUD 查询玩法进度（如 GameSystem 光尘/灯台）
		[[nodiscard]] GameSystem* GetGameSystem();

	private:
		EState m_State = EState::Edit;

		Ref<Level> m_PersistentLevel;   // 编辑源关卡
		Ref<Level> m_RuntimeLevel;      // Play 时由持久关卡复制的运行关卡

		Ref<GameMode> m_GameMode = CreateRef<DefaultGameMode>();
	};
}