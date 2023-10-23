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
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform, sprite.Color);
		}
	}
}