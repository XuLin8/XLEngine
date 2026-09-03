#include "xlpch.h"
#include "GameLayer.h"

#include "GameModule.h"
#include "Systems/GameSystem.h"

#include "Runtime/Core/AppFramework/Application.h"
#include "Runtime/Events/Event.h"
#include "Runtime/Renderer/RenderCommand.h"
#include "Runtime/Renderer/Renderer3D.h"
#include "Runtime/Renderer/TextRenderer.h"
#include "Runtime/Audio/AudioSystem.h"
#include "Runtime/Scene/SceneSerializer.h"
#include "Runtime/Resource/AssetManager/AssetManager.h"

#include <glad/glad.h>

namespace XLEngine
{
	namespace
	{
		// 运行时游玩视角预设（与编辑器 Play 一致：低位跟随轨道相机）
		constexpr float kGameCamDistance = 26.0f;
		constexpr float kGameCamPitch    = 58.0f;

		constexpr glm::vec4 kMoteColor  = { 0.616f, 0.722f, 0.290f, 1.0f }; // 荧绿 #9DB84A
		constexpr glm::vec4 kEmberColor = { 0.788f, 0.431f, 0.227f, 1.0f }; // 余烬橙 #C96E3A
		constexpr glm::vec4 kPaleColor  = { 0.788f, 0.784f, 0.722f, 1.0f }; // 惨白 #C9C8B8
	}

	void GameLayer::OnAttach()
	{
		Window& window = Application::GetInstance().GetWindow();

		m_CameraController = CameraController(30.0f, 1.778f, 0.1f, 1000.0f);
		m_CameraController.SetViewportSize((float)window.GetWidth(), (float)window.GetHeight());

		// P1-1 内置点阵字体（HUD）+ P1-4 合成音频：均零素材
		TextRenderer::Init();
		AudioSystem::Init();

		// 持久关卡 = Meadow.xl（地形/植被/遗迹/灯台锚点，内容数据驱动）
		Ref<Level> level = CreateRef<Level>();
		SceneSerializer sceneLoader(level);
		if (!sceneLoader.Deserialize(AssetManager::GetInstance().GetFullPath("Assets/Scenes/Meadow.xl").string()))
			XL_CORE_ERROR("GameLayer::OnAttach: 加载 Assets/Scenes/Meadow.xl 失败，场景为空");

		m_World.SetPersistentLevel(level);
		// 玩法系统仅注入运行关卡副本（Level::Copy 不复制系统，避免重复装配）
		m_World.SetRuntimeAssembler([](Ref<Level> l) { GameModule::RegisterGameplay(l); });
		level->OnViewportResize(window.GetWidth(), window.GetHeight());

		// 独立游戏：直接进入运行时
		m_World.Play();
		m_ActiveLevel = m_World.GetActiveLevel();
		m_ActiveLevel->OnViewportResize(window.GetWidth(), window.GetHeight());

		// 运行时游玩视角（低位跟随），起始星夜（time=0）
		EditorCamera& cam = m_CameraController.GetCamera();
		cam.SetDistance(kGameCamDistance);
		cam.SetPitch(kGameCamPitch);
		cam.SetFocalPoint(glm::vec3(0.0f, 0.0f, 0.0f));
		Renderer3D::SetTime(0.0f);
	}

	void GameLayer::OnDetach()
	{
		AudioSystem::Shutdown();
	}

	void GameLayer::OnUpdate(Timestep ts)
	{
		Window& window = Application::GetInstance().GetWindow();

		// 渲染到默认帧缓冲：清屏 → 运行时玩法 + 场景 3D
		RenderCommand::SetViewport(0, 0, window.GetWidth(), window.GetHeight());
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		// 运行时低空跟随：焦点每帧钉到玩家位置（镜头相对移动基准也随焦点而动）
		EditorCamera& runtimeCam = m_CameraController.GetCamera();
		if (GameSystem* gs = m_ActiveLevel ? m_ActiveLevel->GetSystem<GameSystem>() : nullptr)
			runtimeCam.SetFocalPoint(gs->GetPlayerPos());

		// World 驱动运行时：GameMode::Tick → 注入运行相机 → Level::OnUpdateRuntime → 玩法 + Render3D
		m_World.Update(ts, &runtimeCam, true);

		// HUD：屏幕空间点阵文字（光尘进度 / 黎明达成 / 操作提示）
		{
			const std::uint32_t W = (std::uint32_t)window.GetWidth();
			const std::uint32_t H = (std::uint32_t)window.GetHeight();

			const GameModule::HudInfo hud = GameModule::QueryHud(m_ActiveLevel);
			if (hud.Active)
			{
				glDisable(GL_DEPTH_TEST); // HUD 置于场景之上
				TextRenderer::BeginScene(W, H);

				// 顶栏：光尘收集进度（惨白）
				std::string motes = "MOTES  " + std::to_string(hud.MotesCollected)
					+ "/" + std::to_string(hud.MotesTotal);
				TextRenderer::DrawString(motes, 14.0f, H - 7.0f * 2.0f - 14.0f, 2.0f, kPaleColor);

				// 黎明达成（顶栏下方，荧绿）
				if (hud.Dawn)
					TextRenderer::DrawString("DAWN  HAS  COME", 14.0f, H - 7.0f * 2.0f * 2.0f - 26.0f, 1.5f, kMoteColor);

				// 底部操作提示（余烬橙）
				TextRenderer::DrawString("WASD  MOVE    E  LIGHT  BEACON", 14.0f, 14.0f, 1.5f, kEmberColor);

				TextRenderer::EndScene();
				glEnable(GL_DEPTH_TEST);
			}
		}
	}

	void GameLayer::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(XL_BIND_EVENT_FN(OnWindowResize));
	}

	bool GameLayer::OnWindowResize(WindowResizeEvent& e)
	{
		m_CameraController.SetViewportSize(e.GetWidth(), e.GetHeight());
		m_World.OnViewportResize(e.GetWidth(), e.GetHeight());
		return false;
	}
}