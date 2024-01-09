#pragma once
#include "Runtime/EcsFramework/Component/ComponentBase.h"

namespace XLEngine
{
	class StaticMeshComponent : public ComponentBase
	{
	public:
		StaticMeshComponent() = default;
		StaticMeshComponent(const StaticMeshComponent&) = default;
		StaticMeshComponent(const std::string& Path)
		{
			
		}
	};
}