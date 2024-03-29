#pragma once

#include "Runtime/Renderer/VertexArray.h"
#include "Runtime/Renderer/VertexBuffer.h"
#include "Runtime/Renderer/IndexBuffer.h"
#include "Runtime/Renderer/Shader.h"

namespace XLEngine
{
	struct Vertex
	{
		glm::vec3 Pos;
		glm::vec3 Normal;
		glm::vec2 Tangent;
		glm::vec2 TexCoord;

		// Editor
		int EntityID;
	};
	
	class StaticMesh
	{
	public:
		StaticMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices);

		void Draw(const glm::mat4& transform, const Ref<Shader>& shader, int entityID);
	private:
		void SetupMesh(int entityID);
	private:
		std::vector<Vertex> mVertices;
		std::vector<uint32_t> mIndices;

		Ref<VertexArray> mVertexArray;
		Ref<VertexBuffer> mVB;
		Ref<IndexBuffer> mIB;

		int mEntityID = -1;
	};
}
