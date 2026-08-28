#pragma once
#include "Runtime/EcsFramework/Component/ComponentBase.h"
#include "Runtime/Renderer/Model.h"

#include <filesystem>
#include <glm/glm.hpp>

namespace XLEngine
{
	class StaticMeshComponent : public ComponentBase
	{
	public:
		StaticMeshComponent() = default;
		StaticMeshComponent(const StaticMeshComponent&) = default;
		StaticMeshComponent(const std::string& path)
			:Path(path)
		{
		}
		StaticMeshComponent(const std::filesystem::path& path)
			:Path(path)
		{
		}
		StaticMeshComponent(const StaticMesh& mesh)
			:Mesh(mesh)
		{
		}

		Model Mesh;
		std::filesystem::path Path;
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
	};
}
