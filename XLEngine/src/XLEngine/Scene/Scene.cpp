#include "xlpch.h"
#include "Scene.h"
#include "Entity.h"

#include "XLEngine/Scene/Component.h"
#include "XLEngine/Renderer/Renderer2D.h"

#include <glm/glm.hpp>

namespace XLEngine
{
	static void DoMath(const glm::mat4& transform)
	{

	}

	// class MeshComponent;

	static void OnTransformConstruct(entt::registry& registry, entt::entity entity)
	{

	}

	Scene::Scene()
	{
#if ENTT_EXAMPLE_CODE 
		entt::entity entity = m_Registry.create();  // 创建一个新的实体
		m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));  // 向实体添加 TransformComponent，并初始化为单位矩阵

		m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();  // 注册 TransformComponent 构造时的回调

		auto view = m_Registry.view<TransformComponent>();  // 获取所有包含 TransformComponent 的实体

		for (auto entity : view)  // 遍历所有包含 TransformComponent 的实体
		{
			TransformComponent& transform = view.get<TransformComponent>(entity);  // 获取 TransformComponent 对象
			// 在这里，你可以执行与 TransformComponent 相关的操作
		}

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);  // 获取所有同时包含 TransformComponent 和 SpriteRendererComponent 的实体

		for (auto entity : group)  // 遍历这个实体组
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);  // 获取 TransformComponent 和 SpriteRendererComponent 对象
			Renderer2D::DrawQuad(transform, sprite.Color);  // 使用这些组件数据进行渲染
		}
#endif
	}

	Scene::~Scene()
	{

	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		// Update scripts
		{
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)  // nsc: native script component
				{
					if (!nsc.Instance)
					{
						nsc.InstantiateFunction();
						nsc.Instance->m_Entity = Entity{ entity, this };
						nsc.OnCreateFunction(nsc.Instance);
					}

					nsc.OnUpdateFunction(nsc.Instance, ts);
				});
		}

		// Remderer 2D
		Camera* mainCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;
		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto& [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = &transform.Transform;
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawQuad(transform, sprite.Color);
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
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
}