#pragma once
#include "Runtime/EcsFramework/Component/ComponentBase.h"
#include "Runtime/Renderer/Model.h"

#include <glm/glm.hpp>

namespace XLEngine
{
	// 程序化散布物件类型（对齐 M0 原型 buildTree / buildRuins / buildRock）
	enum class PropType : int
	{
		TreeStandard = 0,   // 标准锥塔
		TreeBent,           // 风蚀弯折
		TreeRoot,           // 根部裸露
		TreeFallen,         // 倾倒
		RuinsMetal,         // 锈蚀金属板残骸
		RuinsRubble,        // 碎石路基
		Rock                // 岩石
	};

	class PropComponent : public ComponentBase
	{
	public:
		PropComponent() = default;
		PropComponent(const PropComponent&) = default;
		PropComponent(float x, float y, float z, PropType type, float scale)
			: X(x), Y(y), Z(z), Type(type), Scale(scale) {}

		// CPU 生成网格（基础几何体拼接 + 顶点色，零素材），
		// 在 AddComponent 时经 Level::OnComponentAdded 触发。
		// 阶段 E：Y 为内容烘焙进 .xl 的地面高度；引擎只按 (X, Y, Z) 定位，
		// 不再采样具体"岛"高度场（岛布局归 Game/Content）。
		void Generate();

		float X = 0.0f;      // 世界 XZ 定位
		float Y = 0.0f;      // 世界地面高度（内容烘焙）
		float Z = 0.0f;
		PropType Type = PropType::Rock;
		float Scale = 1.0f;

		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };

		Model Mesh;
	};
}
