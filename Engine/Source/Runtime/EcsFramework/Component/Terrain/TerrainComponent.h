#pragma once
#include "Runtime/EcsFramework/Component/ComponentBase.h"
#include "Runtime/Renderer/Model.h"

#include <glm/glm.hpp>

namespace XLEngine
{
	class TerrainComponent : public ComponentBase
	{
	public:
		TerrainComponent() = default;
		TerrainComponent(const TerrainComponent&) = default;

		// CPU 生成地形网格（simplex/fbm 噪声 + 顶点色，零素材）
		void Generate();

		// 生成参数（美术风格锁定，对应 M0 原型默认值）
		float Size = 150.0f;
		uint32_t Segments = 150;
		float HeightScale = 15.0f;
		float NoiseScale = 0.035f;
		uint32_t Seed = 20260829;

		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };

		Model Mesh;
	};
}
