#include "xlpch.h"
#include "Runtime/Renderer/Renderer3D.h"
#include "Runtime/Renderer/VertexArray.h"
#include "Runtime/Renderer/Texture.h"
#include "Runtime/Renderer/Shader.h"
#include "Runtime/Renderer/RenderCommand.h"
#include "Runtime/Resource/AssetManager/AssetManager.h"

#include <glad/glad.h>


namespace XLEngine
{
	static Ref<Shader> mShader;

	// Locked art-direction lighting for the apocalyptic ink style.
	// Values align 1:1 with M0 prototype deep-night (t=0) config in main.js,
	// so the XLEngine toon shader reproduces the exact reference look.
	static void SetSceneLightingUniforms(const glm::vec3& cameraPos)
	{
		mShader->SetFloat3("u_CameraPos", cameraPos);

		// Ambient: #9aa58f * 0.55 (night amb) — faint blue-grey, shadows never pure black
		mShader->SetFloat3("u_AmbientColor", { 0.332f, 0.356f, 0.309f });

		// Sun (moon): direction (-40,24,-30) normalized, cold blue #8aa0ff * 0.9
		mShader->SetFloat3("u_SunDirection", glm::normalize(glm::vec3(-0.72f, 0.43f, -0.54f)));
		mShader->SetFloat3("u_SunColor", { 0.487f, 0.564f, 0.900f });

		// Point light 0: toxic green — M0 (24, h+1.4, -18), h = heightAt(24,-18) = -2.98
		mShader->SetFloat3("u_PL0Pos", { 24.0f, -1.58f, -18.0f });
		mShader->SetFloat3("u_PL0Color", { 0.616f, 0.722f, 0.290f });
		mShader->SetFloat("u_PL0Radius", 46.0f);

		// Point light 1: ember — M0 (-30, h+0.7, 22), h = heightAt(-30,22) = 0.60
		mShader->SetFloat3("u_PL1Pos", { -30.0f, 1.30f, 22.0f });
		mShader->SetFloat3("u_PL1Color", { 0.788f, 0.431f, 0.227f });
		mShader->SetFloat("u_PL1Radius", 26.0f);

		// Exponential fog — M0 #23271c, density tuned for the 135-unit editor
		// camera distance: far ground fades, near stays legible.
		mShader->SetFloat3("u_FogColor", { 0.137f, 0.153f, 0.110f });
		mShader->SetFloat("u_FogDensity", 0.0045f);
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
