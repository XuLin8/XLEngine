#include "xlpch.h"
#include "Runtime/EcsFramework/World/World.h"

namespace XLEngine
{
	void World::SetPersistentLevel(Ref<Level> level)
	{
		// 若正在运行则先回到编辑态，避免替换持久关卡时留下运行副本
		if (m_State == EState::Runtime)
			Stop();
		m_PersistentLevel = level;
	}

	void World::Play()
	{
		if (m_State == EState::Runtime)
			return;
		if (!m_PersistentLevel)
			return;

		m_RuntimeLevel = Level::Copy(m_PersistentLevel);
		m_State = EState::Runtime;
		// 运行关卡装配：由宿主注入玩法系统（引擎不感知具体玩法类型）
		if (m_RuntimeAssembler)
			m_RuntimeAssembler(m_RuntimeLevel);
		if (m_GameMode)
			m_GameMode->BeginPlay(m_RuntimeLevel.get());
	}

	void World::Stop()
	{
		if (m_State != EState::Runtime)
			return;

		if (m_GameMode)
			m_GameMode->EndPlay(m_RuntimeLevel.get());
		m_RuntimeLevel.reset();   // 释放运行关卡，回到编辑源
		m_State = EState::Edit;
	}

	void World::Update(Timestep ts, EditorCamera* camera, bool runtime)
	{
		if (runtime)
		{
			if (m_State == EState::Runtime && m_GameMode && m_RuntimeLevel)
				m_GameMode->Tick(ts, m_RuntimeLevel.get(), camera);
		}
		else
		{
			if (m_State == EState::Edit && m_PersistentLevel && camera)
				m_PersistentLevel->OnUpdateEditor(ts, *camera);
		}
	}

	void World::OnViewportResize(std::uint32_t width, std::uint32_t height)
	{
		if (m_PersistentLevel)
			m_PersistentLevel->OnViewportResize(width, height);
		if (m_RuntimeLevel)
			m_RuntimeLevel->OnViewportResize(width, height);
	}

	Ref<Level> World::GetActiveLevel() const
	{
		if (m_State == EState::Runtime && m_RuntimeLevel)
			return m_RuntimeLevel;
		return m_PersistentLevel;
	}
}