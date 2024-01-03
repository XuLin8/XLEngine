#include "xlpch.h"
#include "Application.h"
#include "Runtime/Core/Log/Log.h"

#include "Runtime/Renderer/Renderer.h"

#include "Input.h"
#include "Runtime/Resource/ConfigManager/ConfigManager.h"

#include <glfw/glfw3.h>

namespace XLEngine
{
	void Application::OnEvent(Event& e)
	{
		XL_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(XL_BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(XL_BIND_EVENT_FN(OnWindowResize));
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend();++it)
		{
			if (e.handled)break;
			(*it)->OnEvent(e);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		XL_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		XL_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::Init(const std::string& name)
	{
		Log::Init();
		XL_CORE_WARN("Initialized Log!");

		m_Window = Window::Create(WindowProps(name));
		m_Window->SetEventCallback(XL_BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		Renderer::Init();
		ConfigManager::GetInstance().Initialize();
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = (float)(glfwGetTime());
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
				for (Layer* layer : m_LayerStack) layer->OnUpdate(timestep);
					
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack) layer->OnImGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
		layer->OnDetach();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::Clean()
	{
		Renderer::Shutdown();

	}
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}
}