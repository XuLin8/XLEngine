#include "xlpch.h"
#include "Runtime/EcsFramework/Entity/Entity.h"

namespace XLEngine
{
	Entity::Entity(entt::entity handle, Level* scene)
		:m_EntityHandle(handle), m_Scene(scene)
	{
		//AddComponent<IDComponent>(UUID());
	}
}