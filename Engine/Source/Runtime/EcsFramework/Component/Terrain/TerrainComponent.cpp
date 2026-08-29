#include "xlpch.h"
#include "Runtime/EcsFramework/Component/Terrain/TerrainComponent.h"
#include "Runtime/Renderer/StaticMesh.h"
#include "Runtime/Utils/Procedural/TerrainNoise.h"

#include <glm/glm.hpp>

#include <cmath>

namespace XLEngine
{
	namespace
	{
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
