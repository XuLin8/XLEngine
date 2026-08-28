#include "xlpch.h"
#include "Runtime/EcsFramework/Component/Terrain/TerrainComponent.h"
#include "Runtime/Renderer/StaticMesh.h"

#include <glm/glm.hpp>

#include <cmath>

namespace XLEngine
{
	namespace
	{
		// 移植自 M0 原型的确定性噪声（与 noise.js 位运算行为一致），
		// 保证 XLEngine 中程序化地形与 M0 视觉完全一致，零素材。
		class TerrainNoise
		{
		public:
			explicit TerrainNoise(uint32_t seed)
			{
				// 32-bit 确定性 PRNG（对齐 JS 的 Math.imul / >>> 语义：
				// 无符号 32 位乘法截断等价于 imul 的位模式，>>> 等价于 uint32 >>）
				auto rand = [s = seed]() mutable -> float {
					s += 0x6d2b79f5u;                                        // (s + 0x6d2b79f5) | 0
					uint32_t t = (s ^ (s >> 15)) * (1u | s);                // imul(s^(s>>>15), 1|s)
					t = (t + ((t ^ (t >> 7)) * (61u | t))) ^ t;             // (t + imul(t^(t>>>7), 61|t)) ^ t
					return (float)(t ^ (t >> 14)) / 4294967296.0f;          // ((t^(t>>>14))>>>0) / 2^32
				};

				uint8_t p[256];
				for (uint32_t i = 0; i < 256; i++)
					p[i] = (uint8_t)i;
				for (uint32_t i = 255; i > 0; i--)
				{
					uint32_t j = (uint32_t)(rand() * (float)(i + 1));
					std::swap(p[i], p[j]);
				}
				for (uint32_t i = 0; i < 512; i++)
					m_Perm[i] = p[i & 255];
			}

			[[nodiscard]] float Noise2(float x, float y) const
			{
				int32_t X = ((int32_t)std::floor(x)) & 255;
				int32_t Y = ((int32_t)std::floor(y)) & 255;
				x -= std::floor(x);
				y -= std::floor(y);
				float u = Fade(x);
				float v = Fade(y);
				int32_t aa = m_Perm[m_Perm[X] + Y];
				int32_t ab = m_Perm[m_Perm[X] + Y + 1];
				int32_t ba = m_Perm[m_Perm[X + 1] + Y];
				int32_t bb = m_Perm[m_Perm[X + 1] + Y + 1];
				return Lerp(
					Lerp(Grad2(aa, x, y), Grad2(ba, x - 1.0f, y), u),
					Lerp(Grad2(ab, x, y - 1.0f), Grad2(bb, x - 1.0f, y - 1.0f), u),
					v);
			}

			[[nodiscard]] float Fbm2(float x, float y, int32_t octaves) const
			{
				float amp = 0.5f;
				float freq = 1.0f;
				float sum = 0.0f;
				float norm = 0.0f;
				for (int32_t i = 0; i < octaves; i++)
				{
					sum += amp * Noise2(x * freq, y * freq);
					norm += amp;
					amp *= 0.5f;
					freq *= 2.0f;
				}
				return sum / norm;
			}

		private:
			static float Fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }
			static float Lerp(float a, float b, float t) { return a + (b - a) * t; }
			static float Grad2(int32_t h, float x, float y)
			{
				int32_t g = h & 7;
				float u = g < 4 ? x : y;
				float v = g < 4 ? y : x;
				return ((g & 1) ? -u : u) + ((g & 2) ? -v : v);
			}

			uint8_t m_Perm[512];
		};

		// 已锁定美术色板（M0 palette）
		constexpr glm::vec3 kMoss    = glm::vec3(0.290f, 0.322f, 0.247f) * 1.4f;  // 苔灰绿（提亮）
		constexpr glm::vec3 kDecay   = glm::vec3(0.290f, 0.227f, 0.173f) * 1.35f; // 朽褐（提亮）
		constexpr glm::vec3 kDark    = glm::vec3(0.169f, 0.165f, 0.145f);         // 暗土褐
		constexpr glm::vec3 kToxic   = glm::vec3(0.616f, 0.722f, 0.290f);         // 荧绿
	}

	void TerrainComponent::Generate()
	{
		TerrainNoise noise(Seed);

		// 高度函数（与 M0 heightAt 一致）
		auto heightAt = [&](float x, float z) -> float {
			const float s = NoiseScale;
			float h = noise.Fbm2(x * s, z * s, 5) * HeightScale;
			h += noise.Noise2(x * s * 3.0f + 17.0f, z * s * 3.0f + 7.0f) * 3.0f;
			return h;
		};

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		const uint32_t seg = Segments;
		const float half = Size * 0.5f;
		const float step = Size / (float)seg;

		// 网格顶点（XZ 平面，Y 为高度）
		for (uint32_t j = 0; j <= seg; j++)
		{
			for (uint32_t i = 0; i <= seg; i++)
			{
				float x = -half + (float)i * step;
				float z = -half + (float)j * step;
				float h = heightAt(x, z);

				Vertex v;
				v.Pos = { x, h, z };

				// 用中心差分算法线（上方向）
				float e = step;
				float hl = heightAt(x - e, z);
				float hr = heightAt(x + e, z);
				float hd = heightAt(x, z - e);
				float hu = heightAt(x, z + e);
				v.Normal = glm::normalize(glm::vec3(hl - hr, 2.0f * e, hd - hu));

				v.Tangent = { 1.0f, 0.0f, 0.0f };
				v.TexCoord = { 0.0f, 0.0f };
				v.EntityID = -1;

				// 顶点色（与 M0 地面着色一致）
				float n = noise.Fbm2(x * 0.02f + 31.0f, z * 0.02f + 11.0f, 3);
				glm::vec3 col;
				if (n > 0.1f)
					col = kMoss;
				else if (n > -0.1f)
					col = glm::mix(kMoss, kDecay, 0.5f);
				else
					col = glm::mix(kDecay, kDark, 0.4f);
				if (h < -2.0f && noise.Noise2(x * 0.1f, z * 0.1f) > 0.6f)
					col = glm::mix(col, kToxic, 0.22f);
				v.Color = glm::vec4(col, 1.0f);

				vertices.push_back(v);
			}
		}

		// 索引（两个三角形 / 格）
		for (uint32_t j = 0; j < seg; j++)
		{
			for (uint32_t i = 0; i < seg; i++)
			{
				uint32_t a = j * (seg + 1) + i;
				uint32_t b = a + 1;
				uint32_t c = (j + 1) * (seg + 1) + i;
				uint32_t d = c + 1;
				indices.push_back(a); indices.push_back(c); indices.push_back(b);
				indices.push_back(b); indices.push_back(c); indices.push_back(d);
			}
		}

		Mesh = Model(StaticMesh(vertices, indices));
	}
}
