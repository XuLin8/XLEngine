#pragma once

#include <cstdint>
#include <string_view>

namespace XLEngine
{
	// 资源大类，与 UE UObject 派生类别映射的最小集合。
	// 目前只有元数据级别的分类，后续会配合具体加载器扩展。
	enum class AssetType : std::uint32_t
	{
		None = 0,
		Scene,       // 关卡场景 (.xl)
		Texture,     // 纹理 (.png/.jpg/.bmp/.tga)
		Shader,      // 着色器 (.glsl/.vert/.frag/.comp)
		Model,       // 模型/网格源 (.obj/.fbx/.gltf/.glb/.dae)
		Font,        // 字体 (.ttf/.otf)
		Audio,       // 音频 (.wav/.mp3/.ogg)
		Material,    // 材质 (.xlmat)
		Other        // 未识别类型
	};

	[[nodiscard]] std::string_view AssetTypeToString(AssetType type);
	[[nodiscard]] AssetType AssetTypeFromString(std::string_view str);
	// 根据文件扩展名推断资源类型
	[[nodiscard]] AssetType AssetTypeFromExtension(std::string_view extension);
}