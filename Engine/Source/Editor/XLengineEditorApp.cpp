#include "XLEngine.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorLayer.h"

namespace XLEngine
{
	void MyAppInitialize(Application& app)
	{
		app.Init("XLEngine Editor");
		app.PushLayer(new EditorLayer());
	}
}