#pragma once

#include "XLEngine/Layer.h"

#include "XLEngine/Events/ApplicationEvent.h"
#include "XLEngine/Events/KeyEvent.h"
#include "XLEngine/Events/MouseEvent.h"


namespace XLEngine {

	class XLENGINE_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();
	private:
		float m_Time = 0.0f;
	};

}