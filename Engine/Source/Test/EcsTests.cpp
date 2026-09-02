#include "xlpch.h"
#include "Test/EngineTest.h"

#include "Runtime/EcsFramework/Level/Level.h"
#include "Runtime/EcsFramework/Entity/Entity.h"
#include "Runtime/EcsFramework/Component/ComponentGroup.h"
#include "Runtime/Scene/SceneSerializer.h"

#include <cstdio>
#include <string>

// 头less ECS 核心测试：仅构造 Level/Entity 并进行纯数据结构操作，
// 不添加 StaticMesh/Terrain/Prop 等需要 GL 上下文的组件，因此无需窗口/渲染环。
namespace XLEngine
{
	namespace
	{
		// 统计场景内带 IDComponent 的实体数（Level::m_Registry 为 public）
		size_t CountEntities(const Level& level)
		{
			size_t n = 0;
			for (auto e : level.m_Registry.view<IDComponent>())
				(void)e, ++n;
			return n;
		}
	}

	TEST(CreateEntityProvidesCoreComponents)
	{
		auto level = CreateRef<Level>();
		Entity e = level->CreateEntity("Player");
		EXPECT_TRUE(e);
		EXPECT_TRUE(e.HasComponent<IDComponent>());
		EXPECT_TRUE(e.HasComponent<TagComponent>());
		EXPECT_TRUE(e.HasComponent<TransformComponent>());
		EXPECT_EQ(e.GetName(), std::string("Player"));

		// 空名回落为 "Entity"
		Entity anon = level->CreateEntity();
		EXPECT_EQ(anon.GetName(), std::string("Entity"));
	}

	TEST(CreateEntityUsesUniqueUUID)
	{
		auto level = CreateRef<Level>();
		Entity a = level->CreateEntity("A");
		Entity b = level->CreateEntity("B");
		EXPECT_TRUE(a.GetUUID() != b.GetUUID());
	}

	TEST(TransformComponentEditedInPlace)
	{
		auto level = CreateRef<Level>();
		Entity e = level->CreateEntity("Hero");
		auto& tc = e.GetComponent<TransformComponent>();
		tc.Translation = glm::vec3(1.5f, 2.5f, 3.5f);
		tc.Rotation = glm::vec3(0.1f, 0.2f, 0.3f);
		tc.Scale = glm::vec3(2.0f, 2.0f, 2.0f);
		EXPECT_NEAR(tc.Translation.x, 1.5f, 1e-4f);
		EXPECT_NEAR(tc.Translation.z, 3.5f, 1e-4f);
		EXPECT_NEAR(tc.Scale.y, 2.0f, 1e-4f);
		// 写回后仍可读回同一值
		EXPECT_NEAR(e.GetComponent<TransformComponent>().Translation.y, 2.5f, 1e-4f);
	}

	TEST(DestroyEntityRemovesFromRegistry)
	{
		auto level = CreateRef<Level>();
		level->CreateEntity("A");
		Entity b = level->CreateEntity("B");
		EXPECT_EQ(CountEntities(*level), 2u);
		level->DestroyEntity(b);
		EXPECT_EQ(CountEntities(*level), 1u);
	}

	TEST(HasComponentAndRemoveComponent)
	{
		auto level = CreateRef<Level>();
		Entity e = level->CreateEntity("X");
		EXPECT_TRUE(e.HasComponent<TransformComponent>());
		e.RemoveComponent<TransformComponent>();
		EXPECT_FALSE(e.HasComponent<TransformComponent>());
	}

	TEST(EntityViewQuery)
	{
		auto level = CreateRef<Level>();
		level->CreateEntity("A");
		level->CreateEntity("B");
		level->CreateEntity("C");

		size_t n = 0;
		for (auto e : level->m_Registry.view<TransformComponent, IDComponent>())
			(void)e, ++n;
		EXPECT_EQ(n, 3u);
	}

	TEST(LevelCopyPreservesStructure)
	{
		auto src = CreateRef<Level>();
		Entity e = src->CreateEntity("Hero");
		e.GetComponent<TransformComponent>().Translation = glm::vec3(4.0f, 5.0f, 6.0f);
		UUID uuid = e.GetUUID();

		Ref<Level> copy = Level::Copy(src);
		EXPECT_EQ(CountEntities(*copy), 1u);

		bool found = false;
		copy->m_Registry.view<IDComponent, TagComponent, TransformComponent>()
			.each([&](entt::entity, IDComponent& id, TagComponent& tag, TransformComponent& tc)
			{
				if (id.ID == uuid)
				{
					found = true;
					EXPECT_EQ(tag.Tag, std::string("Hero"));
					EXPECT_NEAR(tc.Translation.x, 4.0f, 1e-4f);
					EXPECT_NEAR(tc.Translation.y, 5.0f, 1e-4f);
					EXPECT_NEAR(tc.Translation.z, 6.0f, 1e-4f);
				}
			});
		EXPECT_TRUE(found);
	}

	TEST(SceneSerializerRoundTrip)
	{
		auto src = CreateRef<Level>();
		Entity e = src->CreateEntity("Roundtrip");
		e.GetComponent<TransformComponent>().Translation = glm::vec3(1.0f, -2.0f, 7.0f);
		UUID uuid = e.GetUUID();
		auto& cam = e.AddComponent<CameraComponent>();
		cam.Primary = true;
		cam.FixedAspectRatio = false;
		cam.Camera.SetOrthographicSize(10.0f);

		const char* path = "ecs_test_roundtrip.xl"; // 写/读相对测试运行目录（ctest 工作目录）
		SceneSerializer serializer(src);
		serializer.Serialize(path);

		auto dst = CreateRef<Level>();
		SceneSerializer loader(dst);
		EXPECT_TRUE(loader.Deserialize(path));
		EXPECT_EQ(CountEntities(*dst), 1u);

		bool found = false;
		dst->m_Registry.view<IDComponent, TagComponent, TransformComponent>()
			.each([&](entt::entity, IDComponent& id, TagComponent& tag, TransformComponent& tc)
			{
				if (id.ID == uuid)
				{
					found = true;
					EXPECT_EQ(tag.Tag, std::string("Roundtrip"));
					EXPECT_NEAR(tc.Translation.x, 1.0f, 1e-4f);
					EXPECT_NEAR(tc.Translation.y, -2.0f, 1e-4f);
					EXPECT_NEAR(tc.Translation.z, 7.0f, 1e-4f);
				}
			});
		EXPECT_TRUE(found);

		std::remove(path);
	}
}