#pragma once

#include "Runtime/Core/Layer/Layer.h"

#include "Runtime/Events/ApplicationEvent.h"
#include "Runtime/Events/KeyEvent.h"
#include "Runtime/Events/MouseEvent.h"


namespace XLEngine {

	class XLENGINE_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		// UI 帧生命周期：Application 通过 Layer 虚接口驱动，而非持有具体 ImGuiLayer
		virtual void OnImGuiBegin() override;
		virtual void OnImGuiEnd() override;

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};

}