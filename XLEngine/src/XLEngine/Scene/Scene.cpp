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
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform, sprite.Color);
		}
	}
}