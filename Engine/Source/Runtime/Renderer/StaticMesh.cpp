#include "xlpch.h"
#include "Runtime/Renderer/StaticMesh.h"
#include "Runtime/Renderer/RenderCommand.h"

namespace XLEngine
{
	StaticMesh::StaticMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices)
		:mVertices(vertices), mIndices(indices)
	{
		mVertexArray = VertexArray::Create();

		mVB = VertexBuffer::Create(sizeof(Vertex) * vertices.size());
		mVB->SetLayout({
			{ShaderDataType::Float3,	"a_Pos"		},
			{ShaderDataType::Float3,	"a_Noraml"	},
			{ShaderDataType::Float3,	"a_Tangent"	},
			{ShaderDataType::Float2,	"a_TexCoord"},
			{ShaderDataType::Int,		"a_EntityID"},
			});
		mVertexArray->AddVertexBuffer(mVB);
		mIB = IndexBuffer::Create(indices.size());
		mVertexArray->SetIndexBuffer(mIB);
	}

	void StaticMesh::Draw(const glm::mat4& transform, const Ref<Shader>& shader, int entityID)
	{
		SetupMesh(entityID);
		shader->Bind();
		shader->SetMat4("u_Model.Transform", transform);
		mVertexArray->Bind();
		RenderCommand::DrawIndexed(mVertexArray, mIB->GetCount());
	}

	void StaticMesh::SetupMesh(int entityID)
	{
		if (entityID == -1)
		{
			mEntityID = entityID;
			mVertexArray->Bind();

			for (int i = 0; i < mVertices.size(); i++)
				mVertices[i].EntityID = entityID;

			mVB->SetData(mVertices.data(), sizeof(Vertex) * mVertices.size());
			mIB->SetData(mIndices.data(), mIndices.size());

			mVertexArray->Unbind();
		}
	}
}