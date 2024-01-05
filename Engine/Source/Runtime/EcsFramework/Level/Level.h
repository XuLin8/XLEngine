#pragma once

#include "Runtime/Core/Timestep.h"
#include "Runtime/Core/UUID.h"
#include "Runtime/Renderer/EditorCamera.h"

#include <entt.hpp>
#include <unordered_map>

class b2World;

namespace XLEngine
{
	class Entity;

	class Level
	{
	public:
		Level();
		~Level();

		static Ref<Level> Copy(Ref<Level> other);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		void DuplicateEntity(Entity entity);
		
		Entity GetPrimaryCameraEntity();

		template<typename... Componets>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Componets...>();
		}
	public:
		entt::registry m_Registry;

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

	private:
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;
		std::vector<class System*> mSystems;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}