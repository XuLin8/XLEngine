#pragma once
#include "xlpch.h"
#include "Runtime/Core/Base/Base.h"
#include "Window.h"
#include "LayerStack.h"
#include "Runtime/Events/Event.h"
#include "Runtime/Events/ApplicationEvent.h"

#include "Runtime/Core/Timestep.h"
#include "Runtime/ImGui/ImGuiLayer.h"

namespace XLEngine
{
	class Application
	{
	public:
		Application(const std::string& name = "XLEngine");
		virtual ~Application() = default;
		void Init();
		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);

		Window& GetWindow() { return *m_Window; }
		static Application& Get() { return *s_Instance; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
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
	
	//�ڿͻ��˶���
	Application* CreateApplication();
}
