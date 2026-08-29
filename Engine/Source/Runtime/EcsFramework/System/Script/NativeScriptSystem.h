#pragma once
#include "Runtime/EcsFramework/System/System.h"
#include "Runtime/EcsFramework/Level/Level.h"

namespace XLEngine
{
	class NativeScriptSystem : public System
	{
	public:
		NativeScriptSystem(Level* level)
			:System(level)
		{
		}
		virtual ~NativeScriptSystem() = default;
	public:
		void OnRuntimeStart() override;
		void OnRuntimeStop() override;
		void OnUpdateRuntime(Timestep ts) override;
	};
}