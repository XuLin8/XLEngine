#pragma once

#include "Runtime/Core/Base/Base.h"
#include "Runtime/core/Timestep.h"
#include "Runtime/Events/Event.h"

namespace XLEngine
{
	class XLENGINE_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnUpdate(Timestep ts) {};
		virtual void OnImGuiRender() {};
		virtual void OnEvent(Event& event) {};
		
		[[nodiscard]] inline const std::string& GetName()const { return m_DebugName; }

	protected:
		std::string m_DebugName;

	};
}