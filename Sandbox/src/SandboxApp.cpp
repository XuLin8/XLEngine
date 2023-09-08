#include <XLEngine.h>

class Sandbox : public XLEngine::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}

};

XLEngine::Application* XLEngine::CreateApplication()
{
	return new Sandbox();
}