#pragma once

#include "Runtime/Renderer/RenderCommand.h"

#include "Runtime/Renderer/Shader.h"

namespace XLEngine
{
	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();
		static void OnWindowResize(uint32_t width, uint32_t height);
		static void EndScene();

		static void Submit(	const Ref<Shader>& shader,
							const Ref<VertexArray>& vertexArry,
							const glm::mat4& transform = glm::mat4(1.0f));

		[[nodiscard]] inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}