#include "xlpch.h"
#include "Application.h"
#include "Runtime/Core/Log/Log.h"

#include "Runtime/Renderer/Renderer.h"

#include "Runtime/Input/Input.h"
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

		mImGuiLayer = new ImGuiLayer();
		PushOverlay(mImGuiLayer);

		Renderer::Init();
	}

	void Application::Run()
	{
		while (bRunning)
		{
			float time = (float)(glfwGetTime());
			Timestep timestep = time - mLastFrameTime;
			mLastFrameTime = time;

			if (!bMinimized)
				for (Layer* layer : mLayerStack) layer->OnUpdate(timestep);
					
			mImGuiLayer->Begin();
			for (Layer* layer : mLayerStack) layer->OnImGuiRender();
			mImGuiLayer->End();

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