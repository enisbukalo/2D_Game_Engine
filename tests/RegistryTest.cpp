#include <gtest/gtest.h>

#include <Registry.h>
#include <typeindex>

struct Position
{
	int x{0};
};

TEST(RegistryTest, AddRemoveHas)
{
	Registry registry;
	auto     e   = registry.createEntity();

	auto* pos = registry.add<Position>(e, Position{1});
	ASSERT_NE(pos, nullptr);
	EXPECT_EQ(pos->x, 1);
	EXPECT_TRUE(registry.has<Position>(e));

	registry.remove<Position>(e);
	EXPECT_FALSE(registry.has<Position>(e));
}

TEST(RegistryTest, DestroyRemovesComponents)
{
	Registry registry;
	auto     e = registry.createEntity();
	registry.add<Position>(e, Position{5});

	registry.destroy(e);
	EXPECT_FALSE(registry.has<Position>(e));
	EXPECT_FALSE(registry.isAlive(e));
}

TEST(RegistryTest, QueueAddRemove)
{
	Registry registry;
	auto     e = registry.createEntity();

	registry.queueAdd<Position>(e, Position{3});
	registry.flushCommandBuffer();
	EXPECT_TRUE(registry.has<Position>(e));
	auto* pos = registry.get<Position>(e);
	ASSERT_NE(pos, nullptr);
	EXPECT_EQ(pos->x, 3);

	registry.queueRemove<Position>(e);
	registry.flushCommandBuffer();
	EXPECT_FALSE(registry.has<Position>(e));
}

TEST(RegistryTest, DeadEntityGuardsReturnNull)
{
	Registry registry;
	auto     e = registry.createEntity();
	registry.destroy(e);

	EXPECT_EQ(registry.add<Position>(e, Position{2}), nullptr);
	EXPECT_EQ(registry.get<Position>(e), nullptr);
	EXPECT_EQ(registry.tryGet<Position>(e), nullptr);

	// queueAdd on dead entity should be ignored and not resurrect the entity
	registry.queueAdd<Position>(e, Position{9});
	registry.flushCommandBuffer();
	EXPECT_FALSE(registry.has<Position>(e));
}

TEST(RegistryTest, TypeNameRegistration)
{
	Registry registry;
	registry.registerTypeName<Position>("Position");
	EXPECT_EQ(registry.getTypeName<Position>(), "Position");
	EXPECT_EQ(registry.getTypeFromName("Position"), std::type_index(typeid(Position)));

	EXPECT_THROW(registry.registerTypeName<Position>("Other"), std::runtime_error);
}
