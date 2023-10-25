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
		entt::entity entity = m_Registry.create();  // ����һ���µ�ʵ��
		m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));  // ��ʵ����� TransformComponent������ʼ��Ϊ��λ����

		m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();  // ע�� TransformComponent ����ʱ�Ļص�

		auto view = m_Registry.view<TransformComponent>();  // ��ȡ���а��� TransformComponent ��ʵ��

		for (auto entity : view)  // �������а��� TransformComponent ��ʵ��
		{
			TransformComponent& transform = view.get<TransformComponent>(entity);  // ��ȡ TransformComponent ����
			// ����������ִ���� TransformComponent ��صĲ���
		}

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);  // ��ȡ����ͬʱ���� TransformComponent �� SpriteRendererComponent ��ʵ��

		for (auto entity : group)  // �������ʵ����
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);  // ��ȡ TransformComponent �� SpriteRendererComponent ����
			Renderer2D::DrawQuad(transform, sprite.Color);  // ʹ����Щ������ݽ�����Ⱦ
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