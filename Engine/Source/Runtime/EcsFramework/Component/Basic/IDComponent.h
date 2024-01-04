#pragma once
#include "Runtime/EcsFramework/Component/ComponentBase.h"
#include "Runtime/Core/UUID.h"

namespace XLEngine
{
	class IDComponent :public ComponentBase
	{
	public:
		UUID ID;
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const UUID& id)
			:ID(id)
		{

		}
	};
}