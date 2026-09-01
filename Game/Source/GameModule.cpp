#include "xlpch.h"
#include "GameModule.h"

#include "Systems/GameSystem.h"

namespace XLEngine
{
	namespace GameModule
	{
		void RegisterGameplay(Ref<Level> level)
		{
			if (level)
				level->RegisterSystem(new GameSystem(level.get()));
		}

		HudInfo QueryHud(Ref<Level> level)
		{
			HudInfo info;
			if (GameSystem* gs = level ? level->GetSystem<GameSystem>() : nullptr)
			{
				info.Active         = true;
				info.MotesCollected = gs->GetMotesCollected();
				info.MotesTotal     = gs->GetMotesTotal();
				info.BeaconsLit     = gs->GetBeaconsLit();
				info.Dawn           = gs->IsDawn();
			}
			return info;
		}
	}
}