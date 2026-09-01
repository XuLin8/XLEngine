#pragma once
#include "Runtime/EcsFramework/System/System.h"
#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/Renderer/Model.h"
#include "Runtime/Renderer/ParticleSystem.h"

#include <glm/glm.hpp>
#include <vector>

namespace XLEngine
{
	// M2 核心循环：探索 → 收集光尘 → 点亮灯台（×3）→ 黎明时刻。
	// 采用镜头相对移动（WASD/方向键），数据驱动，零素材（程序化 orb 模型）。
	class GameSystem : public System
	{
	public:
		GameSystem(Level* level);
		virtual ~GameSystem() = default;

		void OnRuntimeStart() override;
		void OnUpdateEditor(Timestep ts, EditorCamera& camera) override;
		void OnUpdateRuntime(Timestep ts) override;
		void OnRender3D(EditorCamera& camera) override;

		[[nodiscard]] int GetMotesCollected() const { return m_MotesCollected; }
		[[nodiscard]] int GetMotesTotal() const { return (int)mMotes.size(); }
		[[nodiscard]] int GetBeaconsLit() const { return (int)m_BeaconsLit; }
		[[nodiscard]] bool IsDawn() const { return m_Dawn; }

	private:
		void SpawnLevel();
		void Simulate(Timestep ts, EditorCamera& camera);
		void DrawOrb(const glm::vec3& pos, float radius, const glm::vec4& color);

	private:
		struct Mote { glm::vec3 Pos; float Phase; float Taken; bool Collected = false; };
		struct Beacon { glm::vec3 Pos; float LitTime = -1.0f; };

		Model m_OrbMesh;
		bool m_OrbBuilt = false;
		bool m_Init = false;

		glm::vec3 m_PlayerPos = { 0.0f, 0.0f, 0.0f };
		float m_PlayerSpeed = 16.0f;
		std::vector<Mote> mMotes;
		std::vector<Beacon> mBeacons;

		int m_MotesCollected = 0;
		int m_BeaconsLit = 0;
		bool m_Dawn = false;
		float m_Time = 0.0f;

		bool m_InteractPrev = false;

		// P1-3 粒子反馈（收集爆发 / 点亮爆发 / 光尘漂浮流）
		ParticleSystem m_Particles;

		static constexpr float kCollectRadius = 4.0f;
		static constexpr float kInteractRadius = 6.5f;
		static constexpr float kIslandBound = 64.0f;
		static constexpr float kDawnTime = 0.12f;
	};
}