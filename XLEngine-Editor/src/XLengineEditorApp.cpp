#include "XLEngine.h"
#include "XLEngine/Core/EntryPoint.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorLayer.h"

namespace XLEngine
{
	class XLEngineEditor : public Application
	{
	public:
		XLEngineEditor()
		{
			PushLayer(new EditorLayer());
		}

		~XLEngineEditor()
		{
		}
	};

	Application* CreateApplication()
	{
		return new XLEngineEditor();
	}
}