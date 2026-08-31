#include "xlpch.h"
#include "Runtime/Resource/AssetDescriptor.h"

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <system_error>

namespace XLEngine
{
	bool AssetDescriptor::Serialize(const AssetMetadata& metadata, const std::filesystem::path& descriptorPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		// 统一资产头（与场景序列化内嵌的 XLEngineAsset 结构一致）
		out << YAML::Key << AssetDescriptor::RootKey << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "FormatVersion" << YAML::Value << AssetDescriptor::FormatVersion;
		out << YAML::Key << "Handle" << YAML::Value << metadata.Handle.Get();
		out << YAML::Key << "Type" << YAML::Value << std::string(AssetTypeToString(metadata.Type));
		out << YAML::Key << "RelativePath" << YAML::Value << metadata.RelativePath;
		out << YAML::Key << "Size" << YAML::Value << metadata.Size;
		out << YAML::Key << "Dependencies" << YAML::Value << YAML::BeginSeq;
		for (const AssetHandle& dep : metadata.Dependencies)
			out << dep.Get();
		out << YAML::EndSeq;
		out << YAML::EndMap;
		out << YAML::EndMap;

		std::error_code ec;
		if (descriptorPath.has_parent_path())
			std::filesystem::create_directories(descriptorPath.parent_path(), ec);

		std::ofstream fout(descriptorPath);
		if (!fout)
			return false;
		fout << out.c_str();
		return static_cast<bool>(fout);
	}

	bool AssetDescriptor::Deserialize(AssetMetadata& out, const std::filesystem::path& descriptorPath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(descriptorPath.string());
		}
		catch (...)
		{
			return false;
		}

		YAML::Node node = data[AssetDescriptor::RootKey];
		if (!node)
			return false;

		out.Handle = node["Handle"].as<std::uint64_t>(0);
		out.Type = AssetTypeFromString(node["Type"].as<std::string>("Other"));
		out.RelativePath = node["RelativePath"].as<std::string>();
		out.Size = node["Size"].as<std::uint64_t>(0);
		out.Dependencies.clear();
		for (auto dep : node["Dependencies"])
			out.Dependencies.emplace_back(dep.as<std::uint64_t>(0));

		return out.Handle.IsValid();
	}
}