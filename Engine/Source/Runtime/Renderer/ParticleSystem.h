#pragma once

#include "Runtime/Renderer/Model.h"
#include "Runtime/Renderer/EditorCamera.h"

#include <glm/glm.hpp>
#include <vector>

namespace XLEngine
{
	// P1-3 轻量 CPU 粒子系统：程序化光球（零素材），供光尘漂浮 / 收集 / 点亮等氛围效果。
	// 独立可复用：Update 推进、Render 在 Renderer3D 场景内绘制。
	class ParticleSystem
	{
	public:
		struct Particle
		{
			glm::vec3 Position;
			glm::vec3 Velocity;
			glm::vec4 Color;
			float Size;
			float Lifetime = 1.0f;
			float Age = 0.0f;
		};

		ParticleSystem() = default;

		// 一次性爆发（收集 / 点亮反馈）
		void EmitBurst(const glm::vec3& pos, const glm::vec4& color,
			int count = 10, float speed = 3.0f, float size = 0.32f, float life = 0.9f);

		// 持续喷发（光尘漂浮路径）
		void EmitStream(const glm::vec3& pos, const glm::vec3& baseVel, const glm::vec4& color,
			float rate, float dt, float size = 0.22f, float life = 1.6f);

		void Update(float dt);
		void Render(const EditorCamera& camera);

		void Clear() { m_Particles.clear(); }
		[[nodiscard]] std::size_t Count() const { return m_Particles.size(); }

	private:
		void EnsureOrb();

		std::vector<Particle> m_Particles;
		Model m_OrbMesh;
		bool m_OrbBuilt = false;
		float m_StreamTimer = 0.0f;
	};
}