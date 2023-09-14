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
		if (XLEngine::Input::IsKeyPressed(XL_KEY_TAB))
			XL_TRACE("Tab key is pressed (poll)!");
	}

	void OnEvent(XLEngine::Event& event) override
	{
		//XL_TRACE("{0}", event);
		if (event.GetEventType() == XLEngine::EventType::KeyPressed)
		{
			XLEngine::KeyPressedEvent& e = (XLEngine::KeyPressedEvent&)event;
			if (e.GetKeyCode() == XL_KEY_TAB)
			{
				XL_TRACE("Tab key is pressed (event)!");
			}
			//XL_TRACE("{0}", (char)e.GetKeyCode());
		}
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