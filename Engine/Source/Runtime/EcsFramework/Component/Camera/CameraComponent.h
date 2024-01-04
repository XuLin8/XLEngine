#pragma
#include "Runtime/EcsFramework/Component/ComponentBase.h"
#include "Runtime/Scene/SceneCamera.h"

namespace XLEngine
{
	class CameraComponent :public ComponentBase
	{
	public:
		SceneCamera Camera;
		bool Primary = true;// TODO: think about moving to Scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};
}
