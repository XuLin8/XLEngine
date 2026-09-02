#include "xlpch.h"
#include "EmptyLayer.h"

#include "Runtime/Core/AppFramework/Application.h"
#include "Runtime/Renderer/RenderCommand.h"
#include "Runtime/Renderer/Renderer3D.h"

namespace XLEngine
{
	void EmptyLayer::OnAttach()
	{
		// E3：创建空关卡。不装配任何玩法系统（不调用 GameModule::RegisterGameplay），
		// 仅由引擎 Level 结构驱动编辑态渲染，验证引擎核心可独立运行。
		m_Level = CreateRef<Level>();

		Window& window = Application::GetInstance().GetWindow();
		m_CameraController.SetViewportSize((float)window.GetWidth(), (float)window.GetHeight());
		m_Level->OnViewportResize(window.GetWidth(), window.GetHeight());
	}

	void EmptyLayer::OnDetach()
	{
	}

	void EmptyLayer::OnUpdate(Timestep ts)
	{
		Window& window = Application::GetInstance().GetWindow();

		// 清屏：默认渲染到窗口帧缓冲（无编辑器后处理管线，保持空引擎最小化）
		RenderCommand::SetViewport(0, 0, window.GetWidth(), window.GetHeight());
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		// 空关卡编辑态更新 + 渲染（Level::OnUpdateEditor -> Render3D）
		m_CameraController.OnUpdate(ts);
		m_Level->OnUpdateEditor(ts, m_CameraController.GetCamera());
	}

	void EmptyLayer::OnEvent(Event& e)
	{
		m_CameraController.OnEvent(e);
	}
}