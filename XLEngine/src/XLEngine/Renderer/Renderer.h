#pragma once

#include "RenderCommand.h"

namespace XLEngine
{
	class Renderer
	{
	public:
		static void BeginScene();
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArry);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};
}