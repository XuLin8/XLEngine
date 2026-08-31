#include "xlpch.h"
#include "Runtime/Resource/AssetType.h"

namespace XLEngine
{
	std::string_view AssetTypeToString(AssetType type)
	{
		switch (type)
		{
		case AssetType::None:     return "None";
		case AssetType::Scene:    return "Scene";
		case AssetType::Texture:  return "Texture";
		case AssetType::Shader:   return "Shader";
		case AssetType::Model:    return "Model";
		case AssetType::Font:     return "Font";
		case AssetType::Audio:    return "Audio";
		case AssetType::Material: return "Material";
		case AssetType::Other:    return "Other";
		}
		return "Unknown";
	}

	AssetType AssetTypeFromString(std::string_view str)
	{
		if (str == "Scene")     return AssetType::Scene;
		if (str == "Texture")   return AssetType::Texture;
		if (str == "Shader")    return AssetType::Shader;
		if (str == "Model")     return AssetType::Model;
		if (str == "Font")      return AssetType::Font;
		if (str == "Audio")     return AssetType::Audio;
		if (str == "Material")  return AssetType::Material;
		if (str == "Other")     return AssetType::Other;
		return AssetType::None;
	}

	AssetType AssetTypeFromExtension(std::string_view ext)
	{
		if (ext == ".xl" || ext == ".level")       return AssetType::Scene;
		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg"
			|| ext == ".bmp" || ext == ".tga")     return AssetType::Texture;
		if (ext == ".glsl" || ext == ".vert" || ext == ".frag"
			|| ext == ".comp")                     return AssetType::Shader;
		if (ext == ".obj" || ext == ".fbx" || ext == ".gltf"
			|| ext == ".glb" || ext == ".dae")     return AssetType::Model;
		if (ext == ".ttf" || ext == ".otf")        return AssetType::Font;
		if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") return AssetType::Audio;
		if (ext == ".xlmat")                       return AssetType::Material;
		return AssetType::Other;
	}
}