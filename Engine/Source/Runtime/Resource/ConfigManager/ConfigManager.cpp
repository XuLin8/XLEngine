#include "xlpch.h"
#include "Runtime/Resource/ConfigManager/ConfigManager.h"

namespace XLEngine {
	void ConfigManager::Initialize()
	{
		mRootFolder = { XL_XSTRINGIFY_MACRO(ENGINE_ROOT_DIR) };
		mAssetsFolder = mRootFolder / "Assets";
		mShadersFolder = mRootFolder / "Shaders";
		mResourcesFolder = mRootFolder / "Resources";
	}

	void ConfigManager::Clear()
	{
		mRootFolder.clear();
		mAssetsFolder.clear();
		mShadersFolder.clear();
		mResourcesFolder.clear();
	}

	const std::filesystem::path& ConfigManager::GetRootFolder() const
	{
		XL_CORE_ASSERT(std::filesystem::exists(mRootFolder));
		return mRootFolder;
	}

	const std::filesystem::path& ConfigManager::GetAssetsFolder() const
	{
		XL_CORE_ASSERT(std::filesystem::exists(mAssetsFolder));
		return mAssetsFolder;
	}

	const std::filesystem::path& ConfigManager::GetShadersFolder() const
	{
		XL_CORE_ASSERT(std::filesystem::exists(mShadersFolder));
		return mShadersFolder;
	}

	const std::filesystem::path& ConfigManager::GetResourceFolder() const
	{
		XL_CORE_ASSERT(std::filesystem::exists(mResourcesFolder));
		return mResourcesFolder;
	}

	
}