#include "xlpch.h"
#include "Runtime/Reflection/Reflection.h"

// 组件须在此处可见（类完整），才能用成员指针绑定字段。
#include "Runtime/EcsFramework/Component/ComponentGroup.h"

#include <filesystem>

namespace XLEngine
{
	namespace
	{
		const char* RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType t)
		{
			switch (t)
			{
			case Rigidbody2DComponent::BodyType::Static:    return "Static";
			case Rigidbody2DComponent::BodyType::Dynamic:   return "Dynamic";
			case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
			}
			return "Static";
		}

		Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& s)
		{
			if (s == "Dynamic")  return Rigidbody2DComponent::BodyType::Dynamic;
			if (s == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;
			return Rigidbody2DComponent::BodyType::Static;
		}
	}

	template<>
	TypeInfo BuildComponentTypeInfo<TagComponent>()
	{
		return TypeInfo{ "TagComponent", { XLPROP(TagComponent, Tag) } };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<TransformComponent>()
	{
		return TypeInfo{ "TransformComponent", {
			XLPROP(TransformComponent, Translation),
			XLPROP(TransformComponent, Rotation),
			XLPROP(TransformComponent, Scale),
		} };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<SpriteRendererComponent>()
	{
		// 仅持久化 Color（与历史 .xl 格式保持一致；TilingFactor 未入库）
		return TypeInfo{ "SpriteRendererComponent", {
			XLPROP(SpriteRendererComponent, Color),
		} };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<CircleRendererComponent>()
	{
		// 仅持久化 Color/Thickness/Fade（历史格式未存 Radius）
		return TypeInfo{ "CircleRendererComponent", {
			XLPROP(CircleRendererComponent, Color),
			XLPROP(CircleRendererComponent, Thickness),
			XLPROP(CircleRendererComponent, Fade),
		} };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<Rigidbody2DComponent>()
	{
		return TypeInfo{ "Rigidbody2DComponent", {
			MakeFieldCustom("BodyType",
				[](void* owner, const YAML::Node& value)
				{
					reinterpret_cast<Rigidbody2DComponent*>(owner)->Type =
						RigidBody2DBodyTypeFromString(value.as<std::string>());
				},
				[](YAML::Emitter& out, const void* owner)
				{
					const auto* o = reinterpret_cast<const Rigidbody2DComponent*>(owner);
					out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(o->Type);
				}),
			XLPROP(Rigidbody2DComponent, FixedRotation),
		} };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<BoxCollider2DComponent>()
	{
		return TypeInfo{ "BoxCollider2DComponent", {
			XLPROP(BoxCollider2DComponent, Offset),
			XLPROP(BoxCollider2DComponent, Size),
			XLPROP(BoxCollider2DComponent, Density),
			XLPROP(BoxCollider2DComponent, Friction),
			XLPROP(BoxCollider2DComponent, Restitution),
			XLPROP(BoxCollider2DComponent, RestitutionThreshold),
		} };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<CircleCollider2DComponent>()
	{
		return TypeInfo{ "CircleCollider2DComponent", {
			XLPROP(CircleCollider2DComponent, Offset),
			XLPROP(CircleCollider2DComponent, Radius),
			XLPROP(CircleCollider2DComponent, Density),
			XLPROP(CircleCollider2DComponent, Friction),
			XLPROP(CircleCollider2DComponent, Restitution),
			XLPROP(CircleCollider2DComponent, RestitutionThreshold),
		} };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<StaticMeshComponent>()
	{
		// 仅持久化 Path（历史格式未存 Color；加载顺序由 AddComponent 保证取到 Path）
		return TypeInfo{ "StaticMeshComponent", {
			MakeFieldCustom("Path",
				[](void* owner, const YAML::Node& value)
				{
					reinterpret_cast<StaticMeshComponent*>(owner)->Path = value.as<std::string>();
				},
				[](YAML::Emitter& out, const void* owner)
				{
					const auto* o = reinterpret_cast<const StaticMeshComponent*>(owner);
					out << YAML::Key << "Path" << YAML::Value << o->Path.string();
				}),
		} };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<TerrainComponent>()
	{
		return TypeInfo{ "TerrainComponent", {
			XLPROP(TerrainComponent, Size),
			XLPROP(TerrainComponent, Segments),
			XLPROP(TerrainComponent, HeightScale),
			XLPROP(TerrainComponent, NoiseScale),
			XLPROP(TerrainComponent, Seed),
			XLPROP(TerrainComponent, Color),
		} };
	}

	template<>
	TypeInfo BuildComponentTypeInfo<PropComponent>()
	{
		// Type 为 enum class PropType，MakeField 按底层 int 读写（与历史 .xl 一致）
		return TypeInfo{ "PropComponent", {
			XLPROP(PropComponent, X),
			XLPROP(PropComponent, Y),
			XLPROP(PropComponent, Z),
			XLPROP(PropComponent, Type),
			XLPROP(PropComponent, Scale),
			XLPROP(PropComponent, Color),
		} };
	}
}