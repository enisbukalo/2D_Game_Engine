#include <gtest/gtest.h>

#include <World.h>

namespace
{
struct Position
{
    int x = 0;
    int y = 0;

    Position() = default;
    Position(int px, int py) : x(px), y(py) {}
};
}

TEST(WorldDeferredOrderingTest, QueueAddThenQueueRemoveLastWins)
{
    World  world;
    Entity e = world.createEntity();

    world.queueAdd<Position>(e, 1, 2);
    world.queueRemove<Position>(e);

    world.flushCommandBuffer();

    EXPECT_FALSE(world.has<Position>(e));
    EXPECT_EQ(world.tryGet<Position>(e), nullptr);
}

TEST(WorldDeferredOrderingTest, QueueRemoveThenQueueAddLastWins)
{
    World  world;
    Entity e = world.createEntity();

    world.queueRemove<Position>(e);
    world.queueAdd<Position>(e, 3, 4);

    world.flushCommandBuffer();

    EXPECT_TRUE(world.has<Position>(e));
    const Position* p = world.get<Position>(e);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->x, 3);
    EXPECT_EQ(p->y, 4);
}

TEST(WorldDeferredOrderingTest, QueueAddRemoveAddLastWins)
{
    World  world;
    Entity e = world.createEntity();

    world.queueAdd<Position>(e, 1, 1);
    world.queueRemove<Position>(e);
    world.queueAdd<Position>(e, 9, 10);

    world.flushCommandBuffer();

    EXPECT_TRUE(world.has<Position>(e));
    const Position* p = world.get<Position>(e);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->x, 9);
    EXPECT_EQ(p->y, 10);
}

TEST(WorldDeferredOrderingTest, QueueDestroyThenQueueAddEntityEndsDeadAndDoesNotLeakOnReuse)
{
    World  world;
    Entity e1 = world.createEntity();
    const uint32_t idx = e1.index;

    world.queueDestroy(e1);
    world.queueAdd<Position>(e1, 5, 6);

    world.flushCommandBuffer();

    EXPECT_FALSE(world.isAlive(e1));

    Entity e2 = world.createEntity();
    ASSERT_EQ(e2.index, idx);
    ASSERT_NE(e2.generation, e1.generation);

    EXPECT_FALSE(world.has<Position>(e2));
    EXPECT_EQ(world.tryGet<Position>(e2), nullptr);
}
