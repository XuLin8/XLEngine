#pragma once

#include "Runtime/Utils/Procedural/TerrainNoise.h"

namespace Game
{
	// 内容侧"岛"高度场（阶段 E：具体岛布局归 Content，引擎只保留通用噪声/地形技术）。
	// 该函数与 Meadow.xl 中 TerrainComponent 的生成参数（NoiseScale=0.035、HeightScale=15、
	// Seed=20260829）保持一致，用于玩法角色接地与内容烘焙（PropComponent.Y）。
	// 引擎不再定义任何"岛"布局；改动陆地形态只需调整这里 / .xl 的参数。
	inline float MeadowIslandHeight(float x, float z)
	{
		const XLEngine::TerrainNoise noise(20260829u);
		const float s = 0.035f;
		const float h = noise.Fbm2(x * s, z * s, 5) * 15.0f;
		return h + noise.Noise2(x * s * 3.0f + 17.0f, z * s * 3.0f + 7.0f) * 3.0f;
	}
}