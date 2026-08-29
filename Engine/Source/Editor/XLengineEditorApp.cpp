#include "XLEngine.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Runtime/ImGui/ImGuiLayer.h"
#include "EditorLayer.h"

namespace XLEngine
{
	void MyAppInitialize(Application& app)
	{
		app.Init("XLEngine Editor");

		// 工具层构造具体 UI 实现并注入，Application 仅依赖 Layer 抽象
		ImGuiLayer* imguiLayer = new ImGuiLayer();
		app.SetImGuiLayer(imguiLayer);
		app.PushOverlay(imguiLayer);

		app.PushLayer(new EditorLayer(imguiLayer));
	}
}