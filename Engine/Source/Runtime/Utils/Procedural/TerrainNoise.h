#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace XLEngine
{
	// 确定性 2D 噪声（与 M0 原型 noise.js 位运算语义完全一致），
	// 供地形、程序化物件等共享，保证 XLEngine 与 M0 视觉一致，零素材。
	class TerrainNoise
	{
	public:
		explicit TerrainNoise(uint32_t seed)
		{
			// 32-bit 确定性 PRNG（对齐 JS 的 Math.imul / >>> 语义）
			auto rand = [s = seed]() mutable -> float {
				s += 0x6d2b79f5u;
				uint32_t t = (s ^ (s >> 15)) * (1u | s);
				t = (t + ((t ^ (t >> 7)) * (61u | t))) ^ t;
				return (float)(t ^ (t >> 14)) / 4294967296.0f;
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

	// 与 M0 heightAt 一致的地形高度函数（TERRAIN_SIZE=150 同参数）
	inline float TerrainHeightAt(const TerrainNoise& noise, float x, float z)
	{
		const float s = 0.035f;
		float h = noise.Fbm2(x * s, z * s, 5) * 15.0f;
		h += noise.Noise2(x * s * 3.0f + 17.0f, z * s * 3.0f + 7.0f) * 3.0f;
		return h;
	}
}
