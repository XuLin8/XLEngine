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
	class System;

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

		// 运行模式渲染相机：由 EditorLayer 注入编辑器相机句柄，
		// 供 GameSystem 在运行模式下用作移动方向基准（见 OnUpdateRuntime）。
		[[nodiscard]] EditorCamera* GetRuntimeCamera() { return m_RuntimeCamera; }
		void SetRuntimeCamera(EditorCamera* camera) { m_RuntimeCamera = camera; }

		void DuplicateEntity(Entity entity);
		
		Entity GetPrimaryCameraEntity();

		// 通用系统注册/查询：玩法系统由宿主（装配层）通过 RegisterSystem 注入，
		// 引擎核心不再依赖任何具体玩法类型（Game → Engine 单向依赖）。
		void RegisterSystem(System* system);
		template<typename T>
		T* GetSystem()
		{
			for (System* s : mSystems)
				if (T* t = dynamic_cast<T*>(s))
					return t;
			return nullptr;
		}

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
		// 共享的 3D 渲染流程（编辑/运行共用）：BeginScene → 静态网格/地形/Prop/玩法物件 → EndScene
		void Render3D(EditorCamera& camera);

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;
		std::vector<class System*> mSystems;

		EditorCamera* m_RuntimeCamera = nullptr;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}