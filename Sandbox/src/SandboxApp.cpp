#include <XLEngine.h>

class ExampleLayer :public XLEngine::Layer 
{
public:
	ExampleLayer()
		:Layer("Example")
	{

	}

	void OnUpdate() override
	{
		//XL_INFO("ExampleLayer ::Update");
	}

	void OnEvent(XLEngine::Event& event) override
	{
		XL_TRACE("{0}", event);
	}
};

class Sandbox : public XLEngine::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new XLEngine::ImGuiLayer());
	}

	~Sandbox()
	{

	}

};

XLEngine::Application* XLEngine::CreateApplication()
{
	return new Sandbox();
}