#pragma once
#include "xlpch.h"
#include "Base.h"
#include "Window.h"
#include "LayerStack.h"
#include "XLEngine/Events/Event.h"
#include "XLEngine/Events/ApplicationEvent.h"

#include "XLEngine/Core/Timestep.h"
#include "XLEngine/ImGui/ImGuiLayer.h"

namespace XLEngine
{
	class Application
	{
	public:
		Application(const std::string& name = "XLEngine");
		virtual ~Application() = default;

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		Window& GetWindow() { return *m_Window; }
		static Application& Get() { return *s_Instance; }

		void Close();

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};
	
	//在客户端定义
	Application* CreateApplication();
}
