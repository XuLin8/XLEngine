#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	XL_PROFILE_FUNCTION();

	m_CheckerboardTexture = XLEngine::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
	XL_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(XLEngine::Timestep ts)
{
	XL_PROFILE_FUNCTION();

	// Update
	{
		XL_PROFILE_SCOPE("CameraController::OnUpdate");
		m_CameraController.OnUpdate(ts);
	}

	// Render
	{
		XL_PROFILE_SCOPE("Renderer Prep");
		XLEngine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		XLEngine::RenderCommand::Clear();
	}
	
	{
		XL_PROFILE_SCOPE("Renderer Draw");
		XLEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());
		XLEngine::Renderer2D::DrawRotatedQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, glm::radians(-45.0f), { 0.8f, 0.2f, 0.3f, 1.0f });
		XLEngine::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		XLEngine::Renderer2D::DrawQuad({ 0.0f,  0.0f, -0.1f }, { 10.0f, 10.0f }, m_CheckerboardTexture, 10.0f);
		XLEngine::Renderer2D::EndScene();
	}
	
}

void Sandbox2D::OnImGuiRender()
{
	XL_PROFILE_FUNCTION();
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(XLEngine::Event& e)
{
	m_CameraController.OnEvent(e);
}