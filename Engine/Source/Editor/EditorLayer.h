#pragma once

#include "XLEngine.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

#include "Runtime/ImGui/ImGuiLayer.h"
#include "Runtime/Renderer/CameraController.h"
#include "Runtime/EcsFramework/World/World.h"

namespace XLEngine
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(ImGuiLayer* imguiLayer);
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void OnOverlayRender();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void SerializeScene(Ref<Level> scene, const std::filesystem::path& path);
		void OnScenePlay();
		void OnSceneStop();

		void OnDuplicateEntity();

		// UI Panels
		void UI_Toolbar();
		void LoadDefaultEditorConfig();

		// 玩法装配（装配层职责）：把本项目的玩法系统注册进关卡。
		// 引擎核心不再硬编码任何玩法类，只在本宿主处注入。
		void AttachGameplay(Ref<Level> level);
	private:
		//temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;
		Ref<Framebuffer> m_Framebuffer;

		// Post-processing
		Ref<Framebuffer> m_PostFramebuffer;
		Ref<Shader> m_PostProcessShader;
		Ref<VertexArray> m_ScreenQuadVA;
		
		Ref<Level> m_ActiveScene;
		Ref<Level> m_EditorScene;
		std::filesystem::path m_EditorScenePath;

		// 世界：持有持久关卡(编辑源)与运行关卡，Play/Stop 时在两者间切换并调度 GameMode
		World m_World;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;
		Entity m_HoveredEntity;

		bool m_PrimaryCamera = true;

		CameraController m_EditorCameraController;

		Ref<Texture2D> m_CheckerboardTexture;
		
		bool m_ViewportFocused = false, m_ViewportHovered = false;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		glm::vec2 m_ViewportBounds[2];

		glm::vec4 m_SquareColor = { 0.2f, 0.3f,0.8f,1.0f };

		int m_GizmoType = -1;

		bool m_ShowPhysicsColliders = false;

		// M1 stylized viewport: fullscreen ink output by default, diagnostic split opt-in
		bool m_ShowDiagnostics = false;

		// Day-night cycle (t in [0,1), 0=midnight, 0.5=noon)
		bool m_AutoDayNight = true;
		float m_DayTime = 0.0f;
		float m_DayNightSpeed = 0.015f;

		// FPS free-roam (roam mode): WASD + RMB look, toggled by F / Settings checkbox
		bool m_FlyMode = false;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		// Editor resources
		Ref<Texture2D> m_IconPlay, m_IconStop;

		ImGuiLayer* m_ImGuiLayer = nullptr;   // 由入口注入，工具层合法依赖具体 UI 实现
	};
}
