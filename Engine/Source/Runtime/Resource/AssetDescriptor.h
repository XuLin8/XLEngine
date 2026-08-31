#pragma once

#include "Runtime/Resource/AssetMetadata.h"

#include <filesystem>

namespace XLEngine
{
	// 统一资源描述格式 (URDF, Unified Resource Descriptor Format)。
	// 每个资产旁会生成一个 "<file>.xld" 描述文件，记录：
	//   格式版本 / 句柄 / 类型 / 相对路径 / 大小 / 依赖
	// 编辑器据此加载、预览并管理资产，形成统一描述入口。
	// 场景序列化(.xl)也内嵌同样的 XLEngineAsset 头，从而从"散装 yaml"升级为本格式。
	class AssetDescriptor
	{
	public:
		// 写：把 metadata 落盘为描述文件，返回是否成功。
		static bool Serialize(const AssetMetadata& metadata, const std::filesystem::path& descriptorPath);
		// 读：从描述文件还原 metadata（FilePath 由调用方补齐）。
		static bool Deserialize(AssetMetadata& out, const std::filesystem::path& descriptorPath);

		static constexpr const char* FormatVersion = "1.0";
		// 内嵌到场景/资产文档顶层的统一资产头关键字
		static constexpr const char* RootKey = "XLEngineAsset";
	};
}