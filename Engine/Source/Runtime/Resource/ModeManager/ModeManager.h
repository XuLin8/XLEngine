#pragma once

#include "Runtime/Core/Base/PublicSingleton.h"

namespace XLEngine
{
	// From UE4.27.2
	// TODO
	enum class EditMode
	{
		Select = 0,
		Landscape,
		Foliage,
		BrushEditing,
		MeshPaint,
	};

	class ModeManager final : public PublicSingleton<ModeManager>
	{
	public:
		static bool IsEditState() {
			return ModeManager::GetInstance().IsEditState;
		}
		static void ChangeState()
		{
			ModeManager::GetInstance().bEditState = !ModeManager::GetInstance().bEditState;
		}
	private:
		static bool bEditState;
		static EditMode mEditMode;
	};
}