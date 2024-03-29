#include "xlpch.h"

#include "Runtime/Renderer/IndexBuffer.h"
#include "Runtime/Renderer/Renderer.h"
#include "Runtime/Platform/OpenGL/OpenGLIndexBuffer.h"

namespace XLEngine
{
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			XL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(count);
		}
		XL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			XL_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(indices, count);
		}
		XL_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}