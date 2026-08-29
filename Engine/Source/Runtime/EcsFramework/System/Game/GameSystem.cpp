#include "xlpch.h"
#include "Runtime/EcsFramework/System/Game/GameSystem.h"

#include "Runtime/Renderer/Renderer3D.h"
#include "Runtime/Renderer/StaticMesh.h"
#include "Runtime/Input/InputAction.h"
#include "Runtime/Utils/Procedural/TerrainNoise.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <vector>

namespace XLEngine
{
	namespace
	{
		constexpr float kPi = 3.14159265358979f;

		// 锁定色板（M0）
		constexpr glm::vec4 kMoteColor     = { 0.616f, 0.722f, 0.290f, 1.0f }; // 荧绿 #9DB84A
		constexpr glm::vec4 kEmberColor    = { 0.788f, 0.431f, 0.227f, 1.0f }; // 余烬橙 #C96E3A
		constexpr glm::vec4 kPaleColor     = { 0.788f, 0.784f, 0.722f, 1.0f }; // 惨白 #C9C8B8

		// 确定性 PRNG（mulberry32），保证每局布局一致
		class Rand
		{
		public:
			explicit Rand(uint32_t seed) : m_S(seed) {}
			[[nodiscard]] float Next() // [0,1)
			{
				m_S += 0x6d2b79f5u;
				uint32_t t = (m_S ^ (m_S >> 15)) * (1u | m_S);
				t = (t + ((t ^ (t >> 7)) * (61u | t))) ^ t;
				return (float)(t ^ (t >> 14)) / 4294967296.0f;
			}
		private:
			uint32_t m_S;
		};

		// 低多边形球体（程序化，零素材），白模；由颜色参数按物件染色
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

		float Dist2D(const glm::vec3& a, const glm::vec3& b)
		{
			const float dx = a.x - b.x;
			const float dz = a.z - b.z;
			return std::sqrt(dx * dx + dz * dz);
		}
	}

	GameSystem::GameSystem(Level* level)
		: System(level)
	{
	}

	void GameSystem::OnRuntimeStart()
	{
		if (!m_Init)
		{
			m_Init = true;
			SpawnLevel();
		}
	}

	void GameSystem::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		if (!m_Init)
		{
			m_Init = true;
			SpawnLevel();
		}
		Simulate(ts, camera);
	}

	void GameSystem::SpawnLevel()
	{
		// 最低限字：确保输入动作映射已就绪，并在首次进入时用确定性种子重建一局
		InputActionMapper::Get().LoadDefaultBindings();

		if (!m_OrbBuilt)
		{
			m_OrbMesh = Model(MakeSphere(1.0f, 3, 6)); // 低分辨率「光球」
			m_OrbBuilt = true;
		}

		m_MotesCollected = 0;
		m_BeaconsLit = 0;
		m_Dawn = false;
		m_PlayerPos = { 0.0f, 0.0f, 0.0f };

		TerrainNoise noise(20260829u);
		auto hAt = [&](float x, float z) { return TerrainHeightAt(noise, x, z); };

		// 光尘：确定性散布，远离中心便于引导探索
		Rand rand(20260830u);
		mMotes.clear();
		for (int i = 0; i < 9; i++)
		{
			Mote m;
			do
			{
				m.Pos.x = (rand.Next() - 0.5f) * 150.0f * 0.8f;
				m.Pos.z = (rand.Next() - 0.5f) * 150.0f * 0.8f;
			} while (Dist2D(m.Pos, { 0.0f, 0.0f, 0.0f }) < 18.0f || std::hypot(m.Pos.x, m.Pos.z) > kIslandBound);
			m.Pos.y = hAt(m.Pos.x, m.Pos.z) + 1.2f;
			m.Phase = rand.Next() * 6.28f;
			m.Taken = 0.0f;
			m.Collected = false;
			mMotes.push_back(m);
		}

		// 灯台 ×3：锚定在 M1 遗迹点位附近
		mBeacons.clear();
		const glm::vec3 beaconSpots[3] = {
			{ -20.0f, 0.0f, -8.0f },
			{  13.0f, 0.0f, 12.0f },
			{   2.0f, 0.0f, -20.0f },
		};
		for (int i = 0; i < 3; i++)
		{
			Beacon b;
			b.Pos = beaconSpots[i];
			b.Pos.y = hAt(b.Pos.x, b.Pos.z) + 1.6f;
			b.LitTime = -1.0f;
			mBeacons.push_back(b);
		}
	}

	void GameSystem::Simulate(Timestep ts, EditorCamera& camera)
	{
		m_Time += ts;
		InputActionMapper& input = InputActionMapper::Get();

		TerrainNoise noise(20260829u);
		auto hAt = [&](float x, float z) { return TerrainHeightAt(noise, x, z); };

		// ---- 镜头相对移动（WASD/方向键）----
		glm::vec3 fwd = camera.GetForwardDirection();
		fwd.y = 0.0f;
		if (glm::length(fwd) > 1e-5f) fwd = glm::normalize(fwd);
		glm::vec3 right = camera.GetRightDirection();
		right.y = 0.0f;
		if (glm::length(right) > 1e-5f) right = glm::normalize(right);

		const float axisF = input.GetAxisPolar("MoveForward", "MoveBackward");
		const float axisR = input.GetAxisPolar("MoveRight", "MoveLeft");
		glm::vec3 move = right * axisR + fwd * axisF;
		if (glm::length(move) > 1e-4f)
		{
			move = glm::normalize(move) * (m_PlayerSpeed * ts);
			m_PlayerPos.x = glm::clamp(m_PlayerPos.x + move.x, -kIslandBound, kIslandBound);
			m_PlayerPos.z = glm::clamp(m_PlayerPos.z + move.z, -kIslandBound, kIslandBound);
		}
		m_PlayerPos.y = hAt(m_PlayerPos.x, m_PlayerPos.z) + 0.9f;

		// ---- 收集光尘 ----
		for (auto& mote : mMotes)
		{
			if (mote.Collected)
				continue;
			if (Dist2D(m_PlayerPos, mote.Pos) < kCollectRadius)
			{
				mote.Collected = true;
				m_MotesCollected++;
			}
		}

		// ---- 点亮灯台（E/空格 边沿触发）----
		const bool interact = input.IsPressed("Interact");
		const bool interactPressed = interact && !m_InteractPrev;
		m_InteractPrev = interact;
		for (auto& beacon : mBeacons)
		{
			if (beacon.LitTime < 0.0f && Dist2D(m_PlayerPos, beacon.Pos) < kInteractRadius && interactPressed)
			{
				beacon.LitTime = m_Time;
				m_BeaconsLit++;
			}
		}

		// ---- 黎明时刻：三座灯台全部点亮后，昼夜色调缓慢过渡到黎明 ----
		if (m_BeaconsLit >= (int)mBeacons.size() && !m_Dawn)
			m_Dawn = true;
		if (m_Dawn)
		{
			float cur = Renderer3D::GetTime();
			float next = cur + (kDawnTime - cur) * 0.03f;
			if (std::abs(next - kDawnTime) < 0.0005f) next = kDawnTime;
			Renderer3D::SetTime(glm::clamp(next, 0.0f, 1.0f));
		}
	}

	void GameSystem::DrawOrb(const glm::vec3& pos, float radius, const glm::vec4& color)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos)
			* glm::scale(glm::mat4(1.0f), glm::vec3(radius));
		Renderer3D::DrawModel(transform, m_OrbMesh, color, -1);
	}

	void GameSystem::OnRender3D(EditorCamera& camera)
	{
		if (!m_Init || !m_OrbBuilt)
			return;

		// 玩家标记（余烬橙光球）
		DrawOrb(m_PlayerPos, 1.1f, kEmberColor);

		// 光尘（荧绿漂浮引导，未收集才可见；随收集淡出/消失）
		for (const auto& mote : mMotes)
		{
			if (mote.Collected)
				continue;
			glm::vec3 p = mote.Pos;
			p.y += std::sin(m_Time * 2.0f + mote.Phase) * 0.8f;
			DrawOrb(p, 0.55f, kMoteColor);
		}

		// 灯台：未点亮惨白微光，点亮后余烬橙强光
		for (const auto& beacon : mBeacons)
		{
			if (beacon.LitTime < 0.0f)
				continue; // 未点亮时仅保留场景遗迹本体
			float t = glm::clamp((m_Time - beacon.LitTime) / 1.5f, 0.0f, 1.0f);
			DrawOrb(beacon.Pos, 1.2f + t * 0.8f, kEmberColor);
			DrawOrb(beacon.Pos + glm::vec3(0.0f, 1.4f, 0.0f), 0.7f * t, kPaleColor);
		}
	}
}