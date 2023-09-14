#include "xlpch.h"

#include "WindowsInput.h"

#include "XLEngine/Application.h"
#include <GLFW/glfw3.h>

namespace XLEngine
{
	Input* Input::s_Instance = new WindowsInput();
	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		//����ǿת+�������Զ�������ָ�루���������ָ�룩��������Ӧ�ĳ�Ա����������C++��һ���﷨��
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { float(xpos),float(ypos) };
	}

	float WindowsInput::GetMouseXImpl()
	{
		return GetMousePositionImpl().first;
	}

	float WindowsInput::GetMouseYImpl()
	{
		return GetMousePositionImpl().second;
	}
}