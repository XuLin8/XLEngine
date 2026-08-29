#include "xlpch.h"
#include "Runtime/EcsFramework/System/Script/NativeScriptSystem.h"
#include "Runtime/EcsFramework/Entity/Entity.h"
#include "Runtime/EcsFramework/Entity/ScriptableEntity.h"
#include "Runtime/EcsFramework/Component/ComponentGroup.h"

namespace XLEngine
{
	void NativeScriptSystem::OnRuntimeStart()
	{
		// 一次性实例化所有脚本（并保存其所属实体），避免在更新里反复创建/泄漏
		mLevel->m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			// 已有实例则先销毁（防重复挂载）
			if (nsc.Instance && nsc.DestroyScript)
			{
				nsc.Instance->OnDestory();
				nsc.DestroyScript(&nsc);
			}
			nsc.Instance = nsc.InstantiateScript();
			nsc.Instance->m_Entity = Entity{ entity, mLevel };
			nsc.Instance->OnCreate();
		});
	}

	void NativeScriptSystem::OnUpdateRuntime(Timestep ts)
	{
		mLevel->m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (nsc.Instance)
				nsc.Instance->OnUpdate(ts);
		});
	}

	void NativeScriptSystem::OnRuntimeStop()
	{
		mLevel->m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (nsc.Instance)
			{
				nsc.Instance->OnDestory();
				if (nsc.DestroyScript)
					nsc.DestroyScript(&nsc);
			}
		});
	}
}