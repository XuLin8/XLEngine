#pragma once

#include "XLEngine/Core/Layer.h"

#include "XLEngine/Events/ApplicationEvent.h"
#include "XLEngine/Events/KeyEvent.h"
#include "XLEngine/Events/MouseEvent.h"


namespace XLEngine {

	class XLENGINE_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();
	private:
		float m_Time = 0.0f;
	};

}