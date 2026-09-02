#pragma once

// =============================================================================
// P2 反射系统（核心基础设施）
// -----------------------------------------------------------------------------
// 采用 C++20 模板元编程 + 属性宏，为组件提供类型元数据，从而把场景序列化的
// "手写逐字段 YAML 硬编码" 抽象为统一的反射读写：
//   - FieldDescriptor : 单个可反射字段的读写器（类型擦除，统一遍历）
//   - TypeInfo        : 一个类型的元数据（类型名 + 字段列表）
//   - MakeField       : 通过成员指针自动生成字段读写器（枚举按 int 存储）
//   - MakeFieldCustom : 字段读写完全自定义（用于枚举转字符串等特殊格式）
//   - XLPROP / 集中注册 : 在 ReflectionRegistry.cpp 集中声明各组件字段
//   - SerializeComponent / DeserializeComponent : 反射驱动的通用序列化
// -----------------------------------------------------------------------------
// 依赖：yaml-cpp（读写）+ glm（矩阵/向量）。glm 向量到 YAML 的转换统一放在
// 本头文件（namespace YAML / namespace glm），供任意 TU 直接使用（ADL）。
// =============================================================================

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

#include <functional>
#include <string>
#include <type_traits>
#include <vector>

// ---- glm 向量 -> YAML 节点（node.as<glm::vecN> / node.encode） ----
namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

// ---- glm 向量 -> YAML Emitter（放置在 glm 关联命名空间，启用 ADL） ----
namespace glm
{
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
}

namespace XLEngine
{
	// 单个可反射字段：统一以 void* 访问（类型擦除），便于任意类型遍历。
	struct FieldDescriptor
	{
		using Reader = std::function<void(void* owner, const YAML::Node& value)>;
		using Writer = std::function<void(YAML::Emitter& out, const void* owner)>;

		std::string Name;
		Reader      Read;
		Writer      Write;
	};

	// 一个可反射类型的元数据。
	struct TypeInfo
	{
		std::string Name;
		std::vector<FieldDescriptor> Fields;
	};

	// 通过成员指针自动生成字段读写器。
	// 枚举统一按底层 int 序列化；基本类型 / std::string / glm 向量由 yaml-cpp 处理。
	template< typename Owner, typename T >
	FieldDescriptor MakeField(const char* name, T Owner::* member)
	{
		return FieldDescriptor{
			name,
			[member, name](void* owner, const YAML::Node& value)
			{
				T& ref = (reinterpret_cast<Owner*>(owner)->*member);
				if constexpr (std::is_enum_v<T>)
					ref = static_cast<T>(value.as<int>());
				else
					ref = value.as<T>();
			},
			[member, name](YAML::Emitter& out, const void* owner)
			{
				const T& ref = (reinterpret_cast<const Owner*>(owner)->*member);
				out << YAML::Key << name << YAML::Value;
				if constexpr (std::is_enum_v<T>)
					out << static_cast<int>(ref);
				else
					out << ref;
			}
		};
	}

	// 自定义字段读写（枚举转字符串、嵌套对象等特殊格式）。
	inline FieldDescriptor MakeFieldCustom(std::string name, FieldDescriptor::Reader read, FieldDescriptor::Writer write)
	{
		return FieldDescriptor{ std::move(name), std::move(read), std::move(write) };
	}

	// 反射元数据构建：由各组件在 ReflectionRegistry.cpp 中全特化实现
	//（此处类完整，可安全访问私有/成员指针）。
	template< typename T >
	TypeInfo BuildComponentTypeInfo();

	// 显式特化的前向声明：让其他 TU 避免对主模板的隐式实例化，链接到注册表中的定义。
	// 组件在 Reflection.h 中仅作不完整类型声明（无需完整定义即可使用）。
	template<> TypeInfo BuildComponentTypeInfo<class TagComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class TransformComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class SpriteRendererComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class CircleRendererComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class Rigidbody2DComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class BoxCollider2DComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class CircleCollider2DComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class StaticMeshComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class TerrainComponent>();
	template<> TypeInfo BuildComponentTypeInfo<class PropComponent>();

	// 反射元数据查询入口（线程安全局部静态缓存）。
	template< typename T >
	const TypeInfo& GetTypeInfo()
	{
		static const TypeInfo s_TypeInfo = BuildComponentTypeInfo<T>();
		return s_TypeInfo;
	}

	// 反射驱动的通用序列化：把组件字段写到 YAML（Emitter 已进入该组件的 Map）。
	template< typename T >
	void SerializeComponent(YAML::Emitter& out, const T& component)
	{
		out << YAML::BeginMap;
		for (const auto& field : GetTypeInfo<T>().Fields)
			field.Write(out, &component);
		out << YAML::EndMap;
	}

	// 反射驱动的通用反序列化：从组件节点读回字段（仅覆盖文件中存在的键）。
	template< typename T >
	void DeserializeComponent(T& component, const YAML::Node& componentNode)
	{
		for (const auto& field : GetTypeInfo<T>().Fields)
		{
			const YAML::Node value = componentNode[field.Name];
			if (!value)
				continue;
			field.Read(&component, value);
		}
	}
}

// 生成某个成员指针字段的 FieldDescriptor（供集中注册使用）。
#define XLPROP(Class, member) ::XLEngine::MakeField(#member, &Class::member)