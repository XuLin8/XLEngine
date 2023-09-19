#include <XLEngine.h>
#include <imgui/imgui.h>

class ExampleLayer :public XLEngine::Layer 
{
public:
	ExampleLayer()
		:Layer("Example")
	{

	}

	void OnUpdate() override
	{
		if (XLEngine::Input::IsKeyPressed(XL_KEY_TAB))
			XL_TRACE("Tab key is pressed (poll)!");
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();
	}

	void OnEvent(XLEngine::Event& event) override
	{
		if (event.GetEventType() == XLEngine::EventType::KeyPressed)
		{
			XLEngine::KeyPressedEvent& e = (XLEngine::KeyPressedEvent&)event;
			if (e.GetKeyCode() == XL_KEY_TAB)
			{
				XL_TRACE("Tab key is pressed (event)!");
			}
		}
	}
};

class Sandbox : public XLEngine::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}

};

XLEngine::Application* XLEngine::CreateApplication()
{
	return new Sandbox();
}