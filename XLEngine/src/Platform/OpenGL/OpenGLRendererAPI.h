#pragma once

#include "XLEngine/Renderer/RendererAPI.h"

namespace XLEngine
{
	class OpenGLRendererAPI :public RendererAPI
	{
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
	};
}