#pragma once

#include "Runtime/Renderer/Texture.h"
#include "Runtime/Renderer/SubTexture2D.h"
#include "Runtime/Core/Timestep.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace XLEngine
{
	// P1-1 内置 5x7 点阵位图字体渲染（零依赖、零外部字体资源）。
	// 将字符表烧录到一张纹理，逐字符用 Renderer2D quad 批量绘制。
	// 屏幕空间正交相机（0..width x 0..height），供 HUD / 提示叠加。
	class TextRenderer
	{
	public:
		static void Init();
		static void Shutdown();

		// 用正交 2D 投影打开一帧屏幕空间文本场景
		static void BeginScene(uint32_t viewportWidth, uint32_t viewportHeight);
		static void EndScene();

		// (x,y) 为文本左下角（屏幕/UI 空间，y 向上）。scale = 像素放大倍数。
		static void DrawString(const std::string& text, float x, float y, float scale, const glm::vec4& color);

		// 文本近似像素宽（5 列 + 1 间距）× scale
		[[nodiscard]] static float MeasureWidth(const std::string& text, float scale);

	private:
		static void BuildTexture();

		// 5 列 × 7 行的 5x7 字体（位顺序：bit0 = 顶部行优先，逐列）
		static const uint8_t s_Font[95][5];

		static Ref<Texture2D> s_Atlas;
		static float s_AtlasInvW, s_AtlasInvH;
		static int s_BaseAscii; // 起始 ASCII 码（0x21）
	};
}