#include "xlpch.h"

#include "Runtime/EcsFramework/Entity/Entity.h"
#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/EcsFramework/Component/ComponentGroup.h"
#include "Runtime/EcsFramework/System/SystemGroup.h"
#include "Runtime/Renderer/Renderer2D.h"
#include "Runtime/Renderer/Renderer3D.h"

namespace XLEngine
{
	Level::Level()
	{
		/*mSystems.push_back(new PhysicSystem2D(this));
		mSystems.push_back(new NativeScriptSystem(this));
		mSystems.push_back(new RenderSystem2D(this));*/
	}

	Level::~Level()
	{
		for (auto& system : mSystems)
		{
			delete system;
		}
	}

	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<Component>();
		for (auto srcEntity : view)
		{
			entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

			auto& srcComponent = src.get<Component>(srcEntity);
			dst.emplace_or_replace<Component>(dstEntity, srcComponent);
		}
	}

	template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<Component>())
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	}

	Ref<Level> Level::Copy(Ref<Level> scene)
	{
		Ref<Level> newScene = CreateRef<Level>();

		newScene->m_ViewportWidth = scene->m_ViewportWidth;
		newScene->m_ViewportHeight = scene->m_ViewportHeight;

		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new Level
		auto& srcLevelRegistry = scene->m_Registry;
		auto& dstLevelRegistry = newScene->m_Registry;
		auto idView = srcLevelRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcLevelRegistry.get<IDComponent>(e).ID;
			const auto& name = srcLevelRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		CopyComponent<TransformComponent>(dstLevelRegistry, srcLevelRegistry, enttMap);
		CopyComponent<SpriteRendererComponent>(dstLevelRegistry, srcLevelRegistry, enttMap);
		CopyComponent<CircleRendererComponent>(dstLevelRegistry, srcLevelRegistry, enttMap);
		CopyComponent<CameraComponent>(dstLevelRegistry, srcLevelRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstLevelRegistry, srcLevelRegistry, enttMap);
		CopyComponent<Rigidbody2DComponent>(dstLevelRegistry, srcLevelRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstLevelRegistry, srcLevelRegistry, enttMap);
		CopyComponent<CircleCollider2DComponent>(dstLevelRegistry, srcLevelRegistry, enttMap);

		return newScene;
	}

	Entity Level::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Level::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Level::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Level::OnRuntimeStart()
	{
		for (auto& system : mSystems)
		{
			system->OnRuntimeStart();
		}
	}

	void Level::OnRuntimeStop()
	{
		for (auto& system : mSystems)
		{
			system->OnRuntimeStop();
		}
	}

	void Level::OnUpdateRuntime(Timestep ts)
	{
		for (auto& system : mSystems)
		{
			system->OnUpdateRuntime(ts);
		}
	}

	void Level::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		for (auto& system : mSystems)
		{
			system->OnUpdateEditor(ts,camera);
		}

		Renderer3D::BeginScene(camera);

		auto group = m_Registry.group<TransformComponent>(entt::get<StaticMeshComponent>);

		for (auto entity : group)
		{
			auto [transform, mesh] = group.get<TransformComponent, StaticMeshComponent>(entity);
			Renderer3D::DrawModel(transform.GetTransform(), mesh, (int)entity);
		}
		Renderer3D::EndScene();
	}

	void Level::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportHeight = height;
		m_ViewportWidth = width;

		// Resize our non-FixedAspectRatio Cameras;
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if(!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	void Level::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetName());

		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
		CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
		CopyComponentIfExists<CircleCollider2DComponent>(newEntity, entity);
	}

	Entity Level::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}
		return {};
	}

	template<typename T>
	void Level::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	void Level::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{

	}

	template<>
	void Level::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{

	}

	template<>
	void Level::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Level::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{

	}

	template<>
	void Level::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
	{
	}

	template<>
	void Level::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{

	}

	template<>
	void Level::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{

	}

	template<>
	void Level::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
	{

	}

	template<>
	void Level::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{

	}

	template<>
	void Level::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
	{

	}

	template<>
	void Level::OnComponentAdded<StaticMeshComponent>(Entity entity, StaticMeshComponent& component)
	{
		component.Mesh = Model(component.Path.string());
	}
}