#include "xlpch.h"
#include "Runtime/Renderer/Renderer3D.h"
#include "Runtime/Renderer/VertexArray.h"
#include "Runtime/Renderer/Texture.h"
#include "Runtime/Renderer/Shader.h"
#include "Runtime/Renderer/RenderCommand.h"
#include "Runtime/Resource/AssetManager/AssetManager.h"

#include <glad/glad.h>

#include <cmath>


namespace XLEngine
{
	static Ref<Shader> mShader;

	// Day-night cycle clock: t in [0,1), 0=midnight, 0.5=noon, seamless loop.
	static float m_Time = 0.0f;

	void Renderer3D::SetTime(float time)
	{
		m_Time = time;
	}

	float Renderer3D::GetTime()
	{
		return m_Time;
	}

	// Smooth daylight factor: 0 at midnight, 1 at noon (parabolic, no banding).
	static float DaylightFactor(float t)
	{
		const float x = (t - 0.5f) * 2.0f;   // -1 at midnight, 0 at noon, +1 at midnight
		return glm::clamp(1.0f - x * x, 0.0f, 1.0f);
	}

	// Warm tint near sunrise / sunset (peaks at t≈0.25 and t≈0.75).
	static float WarmFactor(float t)
	{
		const float d0 = 1.0f - std::abs(t - 0.25f) / 0.12f;
		const float d1 = 1.0f - std::abs(t - 0.75f) / 0.12f;
		return glm::clamp(std::max(d0, d1), 0.0f, 1.0f);
	}

	// Art-direction lighting for the apocalyptic ink style.
	// Night base locks 1:1 with the M0 prototype deep-night (t=0) config;
	// the cycle sweeps to a pale apocalyptic daylight and warm dawn/dusk.
	static void SetSceneLightingUniforms(const glm::vec3& cameraPos)
	{
		mShader->SetFloat3("u_CameraPos", cameraPos);

		const float day  = DaylightFactor(m_Time);   // 0 night -> 1 noon
		const float warm = WarmFactor(m_Time);       // sunrise/sunset warm push

		// Night base (M0 deep-night: #9aa58f * 0.55 amb, cold blue #8aa0ff sun)
		const glm::vec3 nightAmb(0.332f, 0.356f, 0.309f);
		const glm::vec3 nightSun(0.487f, 0.564f, 0.900f);
		const glm::vec3 nightFog(0.137f, 0.153f, 0.110f);

		// Day base (pale warm-grey apocalyptic daylight)
		const glm::vec3 dayAmb(0.66f, 0.70f, 0.62f);
		const glm::vec3 daySun(0.90f, 0.84f, 0.68f);
		const glm::vec3 dayFog(0.52f, 0.56f, 0.50f);

		// Dawn/dusk warm sun tint (~#ffc98a)
		const glm::vec3 warmSun(1.00f, 0.79f, 0.54f);

		glm::vec3 amb = glm::mix(nightAmb, dayAmb, day);
		glm::vec3 sun = glm::mix(nightSun, daySun, day);
		sun = glm::mix(sun, warmSun, warm * 0.9f);
		glm::vec3 fog = glm::mix(nightFog, dayFog, day);

		mShader->SetFloat3("u_AmbientColor", amb);

		// Sun arcs upward over the cycle (matches M0 applyTime: -40, 24+50t, -30)
		const float sunY = 24.0f + 60.0f * day;
		mShader->SetFloat3("u_SunDirection", glm::normalize(glm::vec3(-40.0f, sunY, -30.0f)));
		mShader->SetFloat3("u_SunColor", sun);

		// Point light 0: toxic green — M0 (24, h+1.4, -18), h = heightAt(24,-18) = -2.98
		mShader->SetFloat3("u_PL0Pos", { 24.0f, -1.58f, -18.0f });
		mShader->SetFloat3("u_PL0Color", { 0.616f, 0.722f, 0.290f });
		mShader->SetFloat("u_PL0Radius", 46.0f);

		// Point light 1: ember — M0 (-30, h+0.7, 22), h = heightAt(-30,22) = 0.60
		mShader->SetFloat3("u_PL1Pos", { -30.0f, 1.30f, 22.0f });
		mShader->SetFloat3("u_PL1Color", { 0.788f, 0.431f, 0.227f });
		mShader->SetFloat("u_PL1Radius", 26.0f);

		// Exponential fog — night #23271c density 0.0045, day lifts to #848b7c / 0.0032
		mShader->SetFloat3("u_FogColor", fog);
		mShader->SetFloat("u_FogDensity", glm::mix(0.0045f, 0.0032f, day));
	}

	void Renderer3D::Init()
	{
		mShader = Shader::Create(AssetManager::GetInstance().GetFullPath("Shaders/Toon.glsl"));
	}

	void Renderer3D::Shutdown()
	{
	}

	void Renderer3D::DrawModel(const glm::mat4& transform, StaticMeshComponent& MeshComponent, int EntityID)
	{
		mShader->Bind();
		mShader->SetFloat4("u_MaterialColor", MeshComponent.Color);
		MeshComponent.Mesh.Draw(transform, mShader, EntityID);
	}

	void Renderer3D::DrawModel(const glm::mat4& transform, Model& mesh, const glm::vec4& color, int EntityID)
	{
		mShader->Bind();
		mShader->SetFloat4("u_MaterialColor", color);
		mesh.Draw(transform, mShader, EntityID);
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		// Force depth test + write back on: ImGui's backend disables GL_DEPTH_TEST
		// during overlay rendering and relies on apps to restore state.
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);

		mShader->Bind();
		mShader->SetMat4("u_ViewProjection", viewProj);
		SetSceneLightingUniforms(glm::vec3(transform[3]));
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		glm::mat4 viewProj = camera.GetViewProjection();

		mShader->Bind();
		mShader->SetMat4("u_ViewProjection", viewProj);
		SetSceneLightingUniforms(camera.GetPosition());
	}

	void Renderer3D::EndScene()
	{
		mShader->Unbind();
	}

	
}
