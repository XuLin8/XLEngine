#pragma once

#include "Runtime/Core/Base/Base.h"
#include "Runtime/Core/Base/PublicSingleton.h"
#include "Runtime/Core/Window.h"
#include "Runtime/Core/Layer/Layer.h"
#include "Runtime/Core/AppFramework/LayerStack.h"
#include "Runtime/Core/Timestep.h"
#include "Runtime/Events/Event.h"
#include "Runtime/Events/ApplicationEvent.h"

int main(int argc, char** argv);

namespace XLEngine
{
	class Application : public PublicSingleton<Application>
	{
	public:
		Application() = default;
		virtual ~Application() {}

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void PopLayer(Layer* layer);

		// 由工具层注入具体 UI 实现（如 ImGuiLayer）
		void SetImGuiLayer(Layer* layer);

		[[nodiscard]] Window& GetWindow() { return *mWindow; }

		// UI 层（如 ImGuiLayer）通过 Layer 抽象驱动，Application 不依赖具体 UI 实现
		[[nodiscard]] Layer* GetImGuiLayer() { return m_ImGuiLayer; }

		void Close();
	private:
		void Init(const std::string& name);
		void Run();
		void Clean();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		Scope<Window> mWindow;
		Layer* m_ImGuiLayer = nullptr;   // 抽象 UI 层，由工具层注入（如 ImGuiLayer）
		bool bRunning = true;
		bool bMinimized = false;
		LayerStack mLayerStack;
		float mLastFrameTime = 0.0f;
	private:
		friend int ::main(int argc, char** argv);

		// To be defined in CLIENT
		friend void MyAppInitialize(Application& app);
	};
}