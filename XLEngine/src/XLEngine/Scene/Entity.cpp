#include "xlpch.h"
#include "Entity.h"

namespace XLEngine
{
	Entity::Entity(entt::entity handle, Scene* scene)
		:m_EntityHandle(handle), m_Scene(scene)
	{}
}