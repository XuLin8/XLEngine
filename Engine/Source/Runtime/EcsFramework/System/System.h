#pragma once

#include "Runtime/Core/Timestep.h"
#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/Renderer/EditorCamera.h"

namespace XLEngine
{
	class System
	{
	public:
		System() = delete;
		System(Level* level):mLevel(level){}
		virtual ~System() = default;
	public:
		virtual void OnUpdateRuntime(Timestep ts){}
		virtual void OnUpdateEditor(Timestep ts, EditorCamera& camera) {}
		virtual void OnRuntimeStart(){}
		virtual void OnRuntimeStop(){}
	protected:
		Level* mLevel = nullptr;
	};
}