#pragma once

#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/Core/Base/Base.h"

namespace XLEngine
{
	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Level>& level);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);

	private:
		Ref<Level> mLevel;
	};
}