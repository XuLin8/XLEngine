#include "xlpch.h"
#include "Application.h"
#include "Runtime/Core/Log/Log.h"

#include "Runtime/Renderer/Renderer.h"

#include "Runtime/Input/Input.h"
#include "Runtime/Input/InputActionManager.h"
#include "Runtime/Resource/ConfigManager/ConfigManager.h"
#include "Runtime/Utils/PlatformUtils.h"

#include <glfw/glfw3.h>

namespace XLEngine
{
	void Application::OnEvent(Event& e)
	{
		XL_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(XL_BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(XL_BIND_EVENT_FN(OnWindowResize));
		for (auto it = mLayerStack.rbegin(); it != mLayerStack.rend();++it)
		{
			if (e.handled)break;
			(*it)->OnEvent(e);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		XL_PROFILE_FUNCTION();

		mLayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		XL_PROFILE_FUNCTION();

		mLayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::Init(const std::string& name)
	{
		Log::Init();
		XL_CORE_WARN("Initialized Log!");

		ConfigManager::GetInstance().Initialize();

		mWindow = Window::Create(WindowProps(name));
		mWindow->SetEventCallback(XL_BIND_EVENT_FN(Application::OnEvent));

		// 窗口创建后注入到功能层与平台层（Input/FileDialogs），
		// 使这些下层组件不反向依赖 Application 单例。
		Input::SetWindow(mWindow.get());
		FileDialogs::SetOwnerWindow(mWindow->GetNativeWindow());

		// UI 层不再由 Application 创建：由工具层构造具体实现（如 ImGuiLayer）
		// 并通过 SetImGuiLayer + PushOverlay 注入，Application 仅依赖 Layer 抽象
		Renderer::Init();
	}

	void Application::SetImGuiLayer(Layer* layer)
	{
		m_ImGuiLayer = layer;
	}

	void Application::Run()
	{
		while (bRunning)
		{
			float time = (float)(glfwGetTime());
			Timestep timestep = time - mLastFrameTime;
			mLastFrameTime = time;

			// 阶段 D：每帧首部对绑定键做边沿采样，供各层/玩法查询 WasPressed/WasReleased/IsHeld。
			// 放在层更新之前，保证本帧动作查询读到的都是本帧快照。
			InputActionManager::Get().UpdateFrame();

			if (!bMinimized)
				for (Layer* layer : mLayerStack) layer->OnUpdate(timestep);

			if (m_ImGuiLayer) m_ImGuiLayer->OnImGuiBegin();
			for (Layer* layer : mLayerStack) layer->OnImGuiRender();
			if (m_ImGuiLayer) m_ImGuiLayer->OnImGuiEnd();

			mWindow->OnUpdate();
		}
	}

	void Application::PopLayer(Layer* layer)
	{
		mLayerStack.PopLayer(layer);
		layer->OnDetach();
	}

	void Application::Close()
	{
		bRunning = false;
	}

	void Application::Clean()
	{
		Renderer::Shutdown();

	}
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		bRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			bMinimized = true;
			return false;
		}

		bMinimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}