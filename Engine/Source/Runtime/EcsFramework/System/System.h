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
		// 在 Level 的 BeginScene/EndScene 之间调用的 3D 渲染钩子（供玩法物件等在场景中绘制）
		virtual void OnRender3D(EditorCamera& camera) {}
	protected:
		Level* mLevel = nullptr;
	};
}