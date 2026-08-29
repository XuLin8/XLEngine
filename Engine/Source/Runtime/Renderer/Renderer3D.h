#pragma once 
#include "Runtime/Renderer/Camera.h"
#include "Runtime/Renderer/EditorCamera.h"
#include "Runtime/EcsFramework/Component/Mesh/StaticMeshComponent.h"

namespace XLEngine
{
	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		// Day-night cycle: t in [0,1), 0=midnight, 0.5=noon (drives toon lighting)
		static void SetTime(float time);
		static float GetTime();
		static void DrawModel(const glm::mat4& transform, StaticMeshComponent& MeshComponent, int EntityID);
		static void DrawModel(const glm::mat4& transform, Model& mesh, const glm::vec4& color, int EntityID);
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();
	};
}