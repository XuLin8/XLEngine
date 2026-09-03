#pragma once

#include "XLEngine.h"
#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/EcsFramework/World/World.h"
#include "Runtime/Renderer/CameraController.h"

namespace XLEngine
{
	// 独立游戏运行层（打包 exe）：无编辑器 ImGui UI，直接驱动玩法运行时关卡。
	// 装配：持久关卡 = Meadow.xl；一启动即 World.Play() 进入运行时，
	// 复用世界(GameMode/GameSystem)渲染闭环 + 低位跟随相机 + TextRenderer HUD。
	class GameLayer : public Layer
	{
	public:
		GameLayer() = default;
		virtual ~GameLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		void OnEvent(Event& e) override;

	private:
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		CameraController m_CameraController;
		World m_World;
		Ref<Level> m_ActiveLevel;
	};
}