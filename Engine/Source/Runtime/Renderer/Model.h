#pragma once
#include "Runtime/Renderer/Shader.h"

namespace XLEngine
{
	class Model
	{
	public:
		Model() = default;
		Model(const std::string& path)
		{

		}
		void Draw(const glm::mat4& transform, Ref<Shader>& shader, int entityID);
	};
}