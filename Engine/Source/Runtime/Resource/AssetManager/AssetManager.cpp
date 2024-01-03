#include "xlpch.h"
#include "Runtime/Resource/AssetManager/AssetManager.h"
#include "Runtime/Resource/ConfigManager/ConfigManager.h"

namespace XLEngine
{
	std::filesystem::path AssetManager::GetFullPath(const std::string& RelativePath)const
	{
		return ConfigManager::GetInstance().GetRootFolder() / RelativePath;
	}
}