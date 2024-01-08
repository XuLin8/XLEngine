#include "xlpch.h"

#include "Runtime/EcsFramework/System/Render/RenderSystem2D.h"
#include "Runtime/EcsFramework/Component/ComponentGroup.h"
#include "Runtime/EcsFramework/Entity/Entity.h"

#include "Runtime/Renderer/Renderer2D.h"

namespace XLEngine
{
	void RenderSystem2D::OnUpdateRuntime(Timestep ts)
	{
		// Renderer 2D
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = mLevel->m_Registry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			// Draw sprites
			{
				auto group = mLevel->m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto entity : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
				}
			}

			// Draw circle
			{
				auto view = mLevel->m_Registry.view<TransformComponent, CircleRendererComponent>();
				for (auto entity : view)
				{
					auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
				}
			}

			Renderer2D::EndScene();
		}
	}

	void RenderSystem2D::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		
		Renderer2D::BeginScene(camera);

		// Draw sprites
		{
			auto group = mLevel->m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
			}
		}

		// Draw circle
		{
			auto view = mLevel->m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

				Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
			}
		}

		Renderer2D::EndScene();
	}
}