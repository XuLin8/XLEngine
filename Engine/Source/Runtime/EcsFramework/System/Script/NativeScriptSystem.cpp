#include "xlpch.h"
#include "Runtime/EcsFramework/System/Script/NativeScriptSystem.h"
#include "Runtime/Resource/ModeManager/ModeManager.h"
#include "Runtime/EcsFramework/Entity/Entity.h"
#include "Runtime/EcsFramework/Entity/ScriptableEntity.h"
#include "Runtime/EcsFramework/Component/ComponentGroup.h"


namespace XLEngine
{
	void NativeScriptSystem::OnUpdate(Timestep ts)
	{
		// Update scripts
		if (!ModeManager::IsEditState())
		{
			mLevel->m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)  // nsc: native script component
			{
				// TODO: Move to Level::OnScenePlay
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, mLevel };
					nsc.Instance->OnCreate();
				}

				nsc.Instance->OnUpdate(ts);
			});
		}
	}
}