#include "xlpch.h"
#include "Runtime/Renderer/ParticleSystem.h"
#include "Runtime/Renderer/Renderer3D.h"
#include "Runtime/Renderer/StaticMesh.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace XLEngine
{
	namespace
	{
		constexpr float kPi = 3.14159265358979f;

		// 低多边形球体（程序化光球，零素材）
		StaticMesh MakeSphere(float radius, int stacks, int sectors)
		{
			std::vector<Vertex> verts;
			std::vector<uint32_t> idx;
			verts.reserve((stacks + 1) * (sectors + 1));
			for (int i = 0; i <= stacks; i++)
			{
				float phi = (float)i / (float)stacks * kPi;
				float y = radius * std::cos(phi);
				float r = radius * std::sin(phi);
				for (int j = 0; j <= sectors; j++)
				{
					float theta = (float)j / (float)sectors * 2.0f * kPi;
					float x = r * std::cos(theta);
					float z = r * std::sin(theta);
					glm::vec3 n = glm::normalize(glm::vec3(x, y, z));
					Vertex v;
					v.Pos = n * radius;
					v.Normal = n;
					v.Tangent = { 1.0f, 0.0f, 0.0f };
					v.TexCoord = { 0.0f, 0.0f };
					v.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
					v.EntityID = -1;
					verts.push_back(v);
				}
			}
			for (int i = 0; i < stacks; i++)
			{
				for (int j = 0; j < sectors; j++)
				{
					uint32_t first = (uint32_t)(i * (sectors + 1) + j);
					uint32_t second = first + (uint32_t)(sectors + 1);
					idx.push_back(first); idx.push_back(first + 1); idx.push_back(second);
					idx.push_back(second); idx.push_back(first + 1); idx.push_back(second + 1);
				}
			}
			return StaticMesh(verts, idx);
		}
	}

	void ParticleSystem::EnsureOrb()
	{
		if (!m_OrbBuilt)
		{
			m_OrbMesh = Model(MakeSphere(1.0f, 3, 6));
			m_OrbBuilt = true;
		}
	}

	void ParticleSystem::EmitBurst(const glm::vec3& pos, const glm::vec4& color,
		int count, float speed, float size, float life)
	{
		EnsureOrb();
		for (int i = 0; i < count; i++)
		{
			Particle p;
			p.Position = pos;
			p.Velocity = glm::vec3(
				(float)std::sin(i * 2.3f) * speed,
				0.8f * speed + (float)std::cos(i * 1.7f) * 0.4f,
				(float)std::cos(i * 1.3f) * speed);
			p.Color = color;
			p.Size = size;
			p.Lifetime = life;
			p.Age = 0.0f;
			m_Particles.push_back(p);
		}
	}

	void ParticleSystem::EmitStream(const glm::vec3& pos, const glm::vec3& baseVel, const glm::vec4& color,
		float rate, float dt, float size, float life)
	{
		EnsureOrb();
		m_StreamTimer += dt * rate;
		while (m_StreamTimer >= 1.0f)
		{
			m_StreamTimer -= 1.0f;
			Particle p;
			p.Position = pos;
			p.Velocity = baseVel + glm::vec3(
				(float)std::sin(m_Particles.size() * 1.1f) * 0.4f,
				0.2f,
				(float)std::cos(m_Particles.size() * 0.9f) * 0.4f);
			p.Color = color;
			p.Size = size;
			p.Lifetime = life;
			p.Age = 0.0f;
			m_Particles.push_back(p);
		}
	}

	void ParticleSystem::Update(float dt)
	{
		for (auto& p : m_Particles)
		{
			p.Position += p.Velocity * dt;
			p.Age += dt;
		}
		m_Particles.erase(
			std::remove_if(m_Particles.begin(), m_Particles.end(),
				[](const Particle& p) { return p.Age >= p.Lifetime; }),
			m_Particles.end());
	}

	void ParticleSystem::Render(const EditorCamera& camera)
	{
		if (m_Particles.empty())
			return;

		for (const auto& p : m_Particles)
		{
			float t = p.Age / p.Lifetime;
			float alpha = (1.0f - t); // 淡出
			glm::vec4 col = p.Color;
			col.a *= alpha;

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), p.Position)
				* glm::scale(glm::mat4(1.0f), glm::vec3(p.Size));
			Renderer3D::DrawModel(transform, m_OrbMesh, col, -1);
		}
	}
}