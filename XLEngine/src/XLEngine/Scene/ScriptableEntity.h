#pragma once

#include "Entity.h"

namespace XLEngine
{
	class ScriptableEntity
	{
	public:
		template<typename T>
		T& GetComponent()
		{
			return m_Entity.GetComponent<T>();
		}

	private:
		Entity m_Entity;
		friend Scene;
	};
}
