#pragma once

#include "XLEngine.h"
#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/Renderer/CameraController.h"

namespace XLEngine
{
	// 空引擎运行层（E3 验收）：不链接 Game 模块，只驱动引擎 Runtime
	// 渲染一个空关卡，用以验证引擎可脱离玩法内容独立构建并运行。
	class EmptyLayer : public Layer
	{
	public:
		EmptyLayer() = default;
		virtual ~EmptyLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& e) override;

	private:
		CameraController m_CameraController;
		Ref<Level> m_Level;
	};
}