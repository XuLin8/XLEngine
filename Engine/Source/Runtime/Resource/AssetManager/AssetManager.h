#pragma once

#include "Runtime/Core/Base/PublicSingleton.h"

#include <filesystem>

namespace XLEngine
{
	class AssetManager :public PublicSingleton<AssetManager>
	{
	public:
		[[nodiscard]] std::filesystem::path GetFullPath(const std::string& RelativePath) const;
			
	};
}