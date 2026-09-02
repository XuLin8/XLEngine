#include "xlpch.h"
#include "SceneSerializer.h"
#include "Runtime/EcsFramework/Entity/Entity.h"
#include "Runtime/EcsFramework/Component/ComponentGroup.h"
#include "Runtime/Reflection/Reflection.h"
#include "Runtime/Resource/AssetDescriptor.h"
#include "Runtime/Resource/AssetHandle.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

namespace XLEngine
{
	namespace
	{
		// 反射驱动的组件级序列化：写 "组件名: {字段...}"
		template<typename T>
		void SerializeComponentBlock(YAML::Emitter& out, Entity entity)
		{
			const auto& component = entity.GetComponent<T>();
			out << YAML::Key << GetTypeInfo<T>().Name;
			SerializeComponent(out, component);
		}
	}

	SceneSerializer::SceneSerializer(const Ref<Level>& level)
		: mLevel(level)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		XL_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap;// Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
			SerializeComponentBlock<TagComponent>(out, entity);

		if (entity.HasComponent<TransformComponent>())
			SerializeComponentBlock<TransformComponent>(out, entity);

		if (entity.HasComponent<CameraComponent>())
		{
			// Camera 含嵌套 SceneCamera 子地图，字段结构性较强，暂保留手写
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
			SerializeComponentBlock<SpriteRendererComponent>(out, entity);

		if (entity.HasComponent<CircleRendererComponent>())
			SerializeComponentBlock<CircleRendererComponent>(out, entity);

		if (entity.HasComponent<Rigidbody2DComponent>())
			SerializeComponentBlock<Rigidbody2DComponent>(out, entity);

		if (entity.HasComponent<BoxCollider2DComponent>())
			SerializeComponentBlock<BoxCollider2DComponent>(out, entity);

		if (entity.HasComponent<CircleCollider2DComponent>())
			SerializeComponentBlock<CircleCollider2DComponent>(out, entity);

		if (entity.HasComponent<StaticMeshComponent>())
			SerializeComponentBlock<StaticMeshComponent>(out, entity);

		if (entity.HasComponent<TerrainComponent>())
			SerializeComponentBlock<TerrainComponent>(out, entity);

		if (entity.HasComponent<PropComponent>())
			SerializeComponentBlock<PropComponent>(out, entity);

		out << YAML::EndMap;// Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		// 统一资源头（URDF），与 AssetDescriptor 描述的资产头一致，把场景纳入统一格式
		out << YAML::Key << AssetDescriptor::RootKey << YAML::Value;
		out << YAML::BeginMap;
		out << YAML::Key << "FormatVersion" << YAML::Value << AssetDescriptor::FormatVersion;
		out << YAML::Key << "Handle" << YAML::Value << AssetHandleHash(filepath);
		out << YAML::Key << "Type" << YAML::Value << "Scene";
		out << YAML::Key << "RelativePath" << YAML::Value << filepath;
		out << YAML::Key << "Dependencies" << YAML::Value << YAML::BeginSeq << YAML::EndSeq;
		out << YAML::EndMap;

		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
			mLevel->m_Registry.each([&](auto entityID)
				{
					Entity entity = { entityID, mLevel.get() };
					if (!entity)
						return;
					// Serialize Entity
					SerializeEntity(out, entity);
				});
			out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}
	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
	}
	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			return false;
		}

		if (!data["Scene"])
			return false;

		// 读取统一资源头（兼容旧版无头场景文件）
		auto assetNode = data[AssetDescriptor::RootKey];
		if (assetNode)
		{
			AssetHandle handle = assetNode["Handle"].as<std::uint64_t>(0);
			XL_CORE_TRACE("Scene carries unified asset header (Handle={0})", handle.Get());
		}

		std::string sceneName = data["Scene"].as<std::string>();
		XL_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				XL_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = mLevel->CreateEntityWithUUID(uuid, name);

				// Transform 始终存在（CreateEntityWithUUID 已添加），反射读回即可
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					DeserializeComponent(tc, entity["TransformComponent"]);
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				if (auto sprite = entity["SpriteRendererComponent"])
					DeserializeComponent(deserializedEntity.AddComponent<SpriteRendererComponent>(), sprite);

				if (auto circle = entity["CircleRendererComponent"])
					DeserializeComponent(deserializedEntity.AddComponent<CircleRendererComponent>(), circle);

				if (auto rb2d = entity["Rigidbody2DComponent"])
					DeserializeComponent(deserializedEntity.AddComponent<Rigidbody2DComponent>(), rb2d);

				if (auto box2d = entity["BoxCollider2DComponent"])
					DeserializeComponent(deserializedEntity.AddComponent<BoxCollider2DComponent>(), box2d);

				if (auto cc2d = entity["CircleCollider2DComponent"])
					DeserializeComponent(deserializedEntity.AddComponent<CircleCollider2DComponent>(), cc2d);

				// StaticMesh 网格在 OnComponentAdded 时按 Path 加载，须先构造出 Path
				if (auto staticMesh = entity["StaticMeshComponent"])
				{
					std::string path = staticMesh["Path"].as<std::string>();
					deserializedEntity.AddComponent<StaticMeshComponent>(path);
				}

				if (auto terrainComponent = entity["TerrainComponent"])
				{
					auto& terrain = deserializedEntity.AddComponent<TerrainComponent>();
					DeserializeComponent(terrain, terrainComponent);
					// AddComponent 已按默认参数生成；用固化参数重建，保证完全一致
					terrain.Generate();
				}

				if (auto propComponent = entity["PropComponent"])
				{
					auto& prop = deserializedEntity.AddComponent<PropComponent>();
					DeserializeComponent(prop, propComponent);
					// 阶段 E：Y 为内容烘焙进 .xl 的地面高度，引擎按 (X, Y, Z) 直接用固化参数重建
					prop.Generate();
				}
			}
			
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// have not implemented
		XL_CORE_ASSERT(false, "SceneSerializer::DeserializeRuntime");
		return false;
	}
}