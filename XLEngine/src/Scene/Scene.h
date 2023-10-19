#pragma once

#include "entt.hpp"

namespace XLEngine
{
	class Scene
	{
	public:
		Scene();
		~Scene();
	private:
		entt::registry m_Registry;
	};
}