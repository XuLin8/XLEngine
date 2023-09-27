#pragma once

#include "XLEngine.h"

class Sandbox2D :public XLEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(XLEngine::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(XLEngine::Event& e) override;
private:
	XLEngine::OrthographicCameraController m_CameraController;

	//temp
	XLEngine::Ref<XLEngine::VertexArray> m_SquareVA;
	XLEngine::Ref<XLEngine::Shader> m_FlatColorShader;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f,0.8f,1.0f };
};