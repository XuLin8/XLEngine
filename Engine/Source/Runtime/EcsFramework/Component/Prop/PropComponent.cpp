#include "xlpch.h"
#include "Runtime/EcsFramework/Component/Prop/PropComponent.h"
#include "Runtime/Renderer/StaticMesh.h"
#include "Runtime/Utils/Procedural/TerrainNoise.h"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstdint>

namespace XLEngine
{
	namespace
	{
		constexpr float kPi = 3.14159265358979f;

		// ---- 小型确定性 PRNG（mulberry32，种子派生自 X/Z/Type，每株树可复现） ----
		uint32_t MixSeed(float x, float z, int type)
		{
			uint32_t h = (uint32_t)((int32_t)std::floor(x * 10.0f)) * 374761393u
				^ (uint32_t)((int32_t)std::floor(z * 10.0f)) * 668265263u
				^ (uint32_t)type * 2246822519u;
			h = (h ^ (h >> 13)) * 1274126177u;
			return h ^ (h >> 16);
		}

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

		// 以基础几何体拼接生成物件网格（零素材），顶点色烘焙各部分材质色
		struct MeshBuilder
		{
			std::vector<Vertex> Verts;
			std::vector<uint32_t> Idx;

			static Vertex Make(const glm::vec3& p, const glm::vec3& n, const glm::vec3& c)
			{
				Vertex v;
				v.Pos = p;
				v.Normal = n;
				v.Tangent = { 1.0f, 0.0f, 0.0f };
				v.TexCoord = { 0.0f, 0.0f };
				v.Color = glm::vec4(c, 1.0f);
				v.EntityID = -1;
				return v;
			}

			void ApplyTransform(const glm::mat4& m)
			{
				const glm::mat3 nrm = glm::mat3(m);
				for (auto& v : Verts)
				{
					v.Pos = glm::vec3(m * glm::vec4(v.Pos, 1.0f));
					v.Normal = glm::normalize(nrm * v.Normal);
				}
			}

			void Append(const MeshBuilder& other)
			{
				const uint32_t base = (uint32_t)Verts.size();
				for (const auto& v : other.Verts)
					Verts.push_back(v);
				for (uint32_t i : other.Idx)
					Idx.push_back(base + i);
			}

			// 圆柱/圆锥：底心 base，底半径 r0，顶半径 r1（0 即圆锥），高 h，radial 段数
			void AddCylinder(float r0, float r1, float h, int radial, const glm::vec3& col, const glm::vec3& base)
			{
				const uint32_t start = (uint32_t)Verts.size();
				// 侧面（开口朝外）
				for (int i = 0; i <= radial; i++)
				{
					float a = (float)i / (float)radial * 2.0f * kPi;
					float ca = std::cos(a), sa = std::sin(a);
					glm::vec3 n(ca, 0.0f, sa);
					Verts.push_back(Make({ base.x + ca * r0, base.y, base.z + sa * r0 }, n, col));
					Verts.push_back(Make({ base.x + ca * r1, base.y + h, base.z + sa * r1 }, n, col));
				}
				for (int i = 0; i < radial; i++)
				{
					uint32_t a = start + (uint32_t)i * 2;
					uint32_t b = a + 1;
					uint32_t c = a + 2;
					uint32_t d = a + 3;
					Idx.push_back(a); Idx.push_back(b); Idx.push_back(d);
					Idx.push_back(a); Idx.push_back(d); Idx.push_back(c);
				}
				// 底/顶圆盘（r=0 时省略）
				if (r0 > 0.001f) AddDisk(r0, base, radial, false, col);
				if (r1 > 0.001f) AddDisk(r1, { base.x, base.y + h, base.z }, radial, true, col);
			}

			// 沿 Y 的圆盘（up=true 法线朝 +Y，否则朝 -Y）
			void AddDisk(float radius, const glm::vec3& center, int radial, bool up, const glm::vec3& col)
			{
				const glm::vec3 n = up ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, -1.0f, 0.0f);
				const uint32_t centerIdx = (uint32_t)Verts.size();
				Verts.push_back(Make(center, n, col));
				const uint32_t start = (uint32_t)Verts.size();
				for (int i = 0; i <= radial; i++)
				{
					float a = (float)i / (float)radial * 2.0f * kPi;
					Verts.push_back(Make({ center.x + std::cos(a) * radius, center.y, center.z + std::sin(a) * radius }, n, col));
				}
				for (int i = 0; i < radial; i++)
				{
					if (up)
					{
						Idx.push_back(centerIdx);
						Idx.push_back(start + (uint32_t)i + 1);
						Idx.push_back(start + (uint32_t)i);
					}
					else
					{
						Idx.push_back(centerIdx);
						Idx.push_back(start + (uint32_t)i);
						Idx.push_back(start + (uint32_t)i + 1);
					}
				}
			}

			// 长方体：半尺寸 half，中心 center（方向轴对齐，之后可用 ApplyTransform 旋转）
			void AddBox(const glm::vec3& half, const glm::vec3& center, const glm::vec3& col)
			{
				const glm::vec3 v[8] = {
					center + glm::vec3(-half.x, -half.y, -half.z), // 0
					center + glm::vec3( half.x, -half.y, -half.z), // 1
					center + glm::vec3( half.x,  half.y, -half.z), // 2
					center + glm::vec3(-half.x,  half.y, -half.z), // 3
					center + glm::vec3(-half.x, -half.y,  half.z), // 4
					center + glm::vec3( half.x, -half.y,  half.z), // 5
					center + glm::vec3( half.x,  half.y,  half.z), // 6
					center + glm::vec3(-half.x,  half.y,  half.z), // 7
				};
				static const glm::vec3 faceNormals[6] = {
					{ 0.0f, 0.0f,  1.0f}, { 0.0f, 0.0f, -1.0f},
					{ 0.0f,  1.0f, 0.0f}, { 0.0f, -1.0f, 0.0f},
					{ 1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
				};
				static const int faceIdx[6][4] = {
					{ 4, 5, 6, 7 }, // +Z
					{ 1, 0, 3, 2 }, // -Z
					{ 7, 6, 2, 3 }, // +Y
					{ 0, 1, 5, 4 }, // -Y
					{ 5, 1, 2, 6 }, // +X
					{ 0, 4, 7, 3 }, // -X
				};
				for (int f = 0; f < 6; f++)
				{
					const uint32_t baseIdx = (uint32_t)Verts.size();
					for (int k = 0; k < 4; k++)
						Verts.push_back(Make(v[faceIdx[f][k]], faceNormals[f], col));
					Idx.push_back(baseIdx);     Idx.push_back(baseIdx + 1); Idx.push_back(baseIdx + 2);
					Idx.push_back(baseIdx);     Idx.push_back(baseIdx + 2); Idx.push_back(baseIdx + 3);
				}
			}

			// 低多边形岩石：抖动二十面体（对齐 M0 DodecahedronGeometry 的碎块观感）
			void AddRock(float radius, const glm::vec3& col, Rand& rng)
			{
				const float phi = 1.61803398875f;
				glm::vec3 v[12] = {
					{-1.0f,  phi, 0.0f}, { 1.0f,  phi, 0.0f}, {-1.0f, -phi, 0.0f}, { 1.0f, -phi, 0.0f},
					{ 0.0f, -1.0f,  phi}, { 0.0f,  1.0f,  phi}, { 0.0f, -1.0f, -phi}, { 0.0f,  1.0f, -phi},
					{ phi, 0.0f, -1.0f}, { phi, 0.0f,  1.0f}, {-phi, 0.0f, -1.0f}, {-phi, 0.0f,  1.0f},
				};
				for (auto& p : v)
				{
					float j = 1.0f + (rng.Next() - 0.5f) * 0.5f;
					p = glm::normalize(p) * radius * j;
				}
				static const int faces[20][3] = {
					{ 0, 11, 5 }, { 0, 5, 1 }, { 0, 1, 7 }, { 0, 7, 10 }, { 0, 10, 11 },
					{ 1, 5, 9 }, { 5, 11, 4 }, { 11, 10, 2 }, { 10, 7, 6 }, { 7, 1, 8 },
					{ 3, 9, 4 }, { 3, 4, 2 }, { 3, 2, 6 }, { 3, 6, 8 }, { 3, 8, 9 },
					{ 4, 9, 5 }, { 2, 4, 11 }, { 6, 2, 10 }, { 8, 6, 7 }, { 9, 8, 1 },
				};
				for (int f = 0; f < 20; f++)
				{
					const glm::vec3& a = v[faces[f][0]];
					const glm::vec3& b = v[faces[f][1]];
					const glm::vec3& c = v[faces[f][2]];
					glm::vec3 n = glm::cross(b - a, c - a);
					if (glm::dot(n, a) < 0.0f)
						n = -n;
					n = glm::normalize(n);
					const uint32_t baseIdx = (uint32_t)Verts.size();
					Verts.push_back(Make(a, n, col));
					Verts.push_back(Make(b, n, col));
					Verts.push_back(Make(c, n, col));
					Idx.push_back(baseIdx);     Idx.push_back(baseIdx + 1); Idx.push_back(baseIdx + 2);
				}
			}
		};
	}

	void PropComponent::Generate()
	{
		TerrainNoise noise(20260829u);
		const float ground = TerrainHeightAt(noise, X, Z);

		const float s = Scale;
		MeshBuilder mb;
		Rand rng(MixSeed(X, Z, (int)Type));

		// M0 物件色板（锁定）
		const glm::vec3 trunkCol(0.165f, 0.173f, 0.133f);   // #2a2c22
		const glm::vec3 rockCol(0.169f, 0.176f, 0.149f);    // #2b2d26
		const glm::vec3 rustCol(0.290f, 0.227f, 0.173f);    // #4a3a2c 朽褐锈蚀
		const glm::vec3 metalCol(0.208f, 0.220f, 0.196f);   // #353832 锈蚀金属深灰
		static const glm::vec3 foliageCols[3] = {
			{ 0.102f, 0.110f, 0.094f },   // #1a1c18
			{ 0.114f, 0.129f, 0.102f },   // #1d211a
			{ 0.133f, 0.125f, 0.094f },   // #222018
		};
		const glm::vec3 foliageCol = foliageCols[(int)(rng.Next() * 3.0f)];

		switch (Type)
		{
		case PropType::TreeStandard:
		{
			mb.AddCylinder(0.50f * s, 0.35f * s, 2.40f * s, 5, trunkCol, { 0.0f, ground, 0.0f });
			mb.AddCylinder(2.30f * s, 0.0f, 3.40f * s, 5, foliageCol, { 0.0f, ground + 1.20f * s, 0.0f });
			mb.AddCylinder(1.70f * s, 0.0f, 2.50f * s, 5, foliageCol, { 0.0f, ground + 3.35f * s, 0.0f });
			mb.AddCylinder(1.00f * s, 0.0f, 1.90f * s, 5, foliageCol, { 0.0f, ground + 5.15f * s, 0.0f });
			break;
		}
		case PropType::TreeBent:
		{
			const float bend = (rng.Next() - 0.5f) * 0.5f;
			float prevX = 0.0f;
			for (int i = 0; i < 3; i++)
			{
				const float r0 = (0.5f - (float)i * 0.13f) * s;
				const float r1 = std::max(r0 * 0.7f, 0.08f);
				const float yOff = ((float)i + 0.55f) * 1.15f * s;
				const float xOff = prevX + bend * (float)(i + 1) * s * 0.5f;
				MeshBuilder seg;
				seg.AddCylinder(r0, r1, 1.15f * s, 5, trunkCol, { 0.0f, 0.0f, 0.0f });
				const glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(xOff, ground + yOff, 0.0f))
					* glm::rotate(glm::mat4(1.0f), bend * (float)(i + 1) * 0.7f, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.575f * s, 0.0f));
				seg.ApplyTransform(m);
				mb.Append(seg);
				prevX = xOff;
			}
			MeshBuilder crown;
			crown.AddCylinder(1.80f * s, 0.0f, 2.60f * s, 5, foliageCol, { 0.0f, 0.0f, 0.0f });
			const glm::mat4 cm = glm::translate(glm::mat4(1.0f), glm::vec3(prevX + bend * s, ground + 4.40f * s, 0.0f))
				* glm::rotate(glm::mat4(1.0f), bend * 1.6f, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.30f * s, 0.0f));
			crown.ApplyTransform(cm);
			mb.Append(crown);
			break;
		}
		case PropType::TreeRoot:
		{
			mb.AddCylinder(0.50f * s, 0.32f * s, 2.60f * s, 5, trunkCol, { 0.0f, ground, 0.0f });
			for (int i = 0; i < 5; i++)
			{
				const float a = (float)i / 5.0f * 2.0f * kPi + rng.Next() * 0.6f;
				MeshBuilder root;
				root.AddCylinder(0.15f * s, 0.06f * s, 1.20f * s, 4, trunkCol, { 0.0f, 0.0f, 0.0f });
				const glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(std::cos(a) * 0.55f * s, ground + 0.35f * s, std::sin(a) * 0.55f * s))
					* glm::rotate(glm::mat4(1.0f), std::cos(a) * 0.9f, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), -std::sin(a) * 0.9f, glm::vec3(1.0f, 0.0f, 0.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.60f * s, 0.0f));
				root.ApplyTransform(m);
				mb.Append(root);
			}
			mb.AddCylinder(2.10f * s, 0.0f, 3.00f * s, 5, foliageCol, { 0.0f, ground + 1.70f * s, 0.0f });
			mb.AddCylinder(1.50f * s, 0.0f, 2.20f * s, 5, foliageCol, { 0.0f, ground + 3.80f * s, 0.0f });
			break;
		}
		case PropType::TreeFallen:
		{
			MeshBuilder trunk;
			trunk.AddCylinder(0.42f * s, 0.30f * s, 4.60f * s, 5, trunkCol, { 0.0f, 0.0f, 0.0f });
			const glm::mat4 tm = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, ground + 0.45f * s, 0.0f))
				* glm::rotate(glm::mat4(1.0f), 0.5f * kPi, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.30f * s, 0.0f));
			trunk.ApplyTransform(tm);
			mb.Append(trunk);
			mb.AddCylinder(1.90f * s, 0.0f, 2.40f * s, 5, foliageCol, { 2.10f * s, ground - 0.65f * s, 0.0f });
			mb.AddCylinder(1.10f * s, 0.0f, 1.70f * s, 5, foliageCol, { 3.20f * s, ground - 0.35f * s, 0.0f });
			break;
		}
		case PropType::RuinsMetal:
		{
			MeshBuilder plate;
			plate.AddBox({ 1.60f, 0.175f, 1.05f }, { 0.0f, 0.0f, 0.0f }, rustCol);
			const glm::mat4 pm = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, ground + 0.18f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), rng.Next() * kPi, glm::vec3(0.0f, 1.0f, 0.0f));
			plate.ApplyTransform(pm);
			mb.Append(plate);

			MeshBuilder pillar;
			pillar.AddBox({ 0.25f, 1.20f, 0.25f }, { 0.0f, 0.0f, 0.0f }, metalCol);
			const glm::mat4 qm = glm::translate(glm::mat4(1.0f), glm::vec3(1.10f, ground + 1.35f, 0.70f))
				* glm::rotate(glm::mat4(1.0f), 0.12f, glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), 0.18f, glm::vec3(0.0f, 0.0f, 1.0f));
			pillar.ApplyTransform(qm);
			mb.Append(pillar);
			break;
		}
		case PropType::RuinsRubble:
		{
			for (int i = 0; i < 5; i++)
			{
				const float ss = 0.5f + rng.Next() * 0.7f;
				MeshBuilder rb;
				rb.AddRock(ss, rustCol, rng);
				const glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3((float)(i - 2) * 1.15f, ground + ss * 0.3f, (rng.Next() - 0.5f) * 0.7f))
					* glm::rotate(glm::mat4(1.0f), rng.Next() * kPi, glm::vec3(1.0f, 0.0f, 0.0f))
					* glm::rotate(glm::mat4(1.0f), rng.Next() * kPi, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::rotate(glm::mat4(1.0f), rng.Next() * kPi, glm::vec3(0.0f, 1.0f, 0.0f));
				rb.ApplyTransform(m);
				mb.Append(rb);
			}
			break;
		}
		case PropType::Rock:
		default:
		{
			MeshBuilder rk;
			rk.AddRock(1.0f, rockCol, rng);
			const glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, ground + 0.15f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), rng.Next() * kPi, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::rotate(glm::mat4(1.0f), rng.Next() * kPi, glm::vec3(0.0f, 0.0f, 1.0f))
				* glm::rotate(glm::mat4(1.0f), rng.Next() * kPi, glm::vec3(1.0f, 0.0f, 0.0f))
				* glm::scale(glm::mat4(1.0f), glm::vec3(s, s * 0.55f, s));
			rk.ApplyTransform(m);
			mb.Append(rk);
			break;
		}
		}

		// 世界坐标烘焙：此前仅用 X/Z 采样地面高度、未做水平偏移，导致所有物件堆在地图中心 (0,*,0)。
		// 统一平移到世界坐标 (X, ground, Z)（与 TerrainComponent 一致，模型使用单位变换渲染）。
		const glm::mat4 world = glm::translate(glm::mat4(1.0f), glm::vec3(X, 0.0f, Z));
		mb.ApplyTransform(world);

		Mesh = Model(StaticMesh(mb.Verts, mb.Idx));
	}
}
