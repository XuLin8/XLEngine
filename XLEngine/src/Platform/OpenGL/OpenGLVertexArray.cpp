#include "xlpch.h"

#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace XLEngine
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case XLEngine::ShaderDataType::Float:		return GL_FLOAT;
		case XLEngine::ShaderDataType::Float2:		return GL_FLOAT;
		case XLEngine::ShaderDataType::Float3:		return GL_FLOAT;
		case XLEngine::ShaderDataType::Float4:		return GL_FLOAT;
		case XLEngine::ShaderDataType::Mat3:		return GL_FLOAT;
		case XLEngine::ShaderDataType::Mat4:		return GL_FLOAT;
		case XLEngine::ShaderDataType::Int:			return GL_INT;
		case XLEngine::ShaderDataType::Int2:		return GL_INT;
		case XLEngine::ShaderDataType::Int3:		return GL_INT;
		case XLEngine::ShaderDataType::Int4:		return GL_INT;
		case XLEngine::ShaderDataType::Bool:		return GL_BOOL;
		}
		XL_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		XL_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		XL_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		XL_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		XL_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		XL_PROFILE_FUNCTION();

		XL_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		uint32_t index = 0;
		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			index++;
		}
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		XL_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}