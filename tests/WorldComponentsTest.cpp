#include <gtest/gtest.h>

#include <World.h>

#include <algorithm>
#include <typeindex>
#include <vector>

namespace
{
struct Position
{
    int x = 0;
    int y = 0;

    Position() = default;
    Position(int px, int py) : x(px), y(py) {}
};

struct Velocity
{
    int dx = 0;
    int dy = 0;

    Velocity() = default;
    Velocity(int vx, int vy) : dx(vx), dy(vy) {}
};
}

TEST(WorldComponentsTest, AddHasGetRemoveRoundTrip)
{
    World  world;
    Entity e = world.createEntity();

    EXPECT_FALSE(world.has<Position>(e));
    EXPECT_EQ(world.get<Position>(e), nullptr);
    EXPECT_EQ(world.tryGet<Position>(e), nullptr);

    Position* p = world.add<Position>(e, 1, 2);
    ASSERT_NE(p, nullptr);
    EXPECT_TRUE(world.has<Position>(e));

    Position* got = world.get<Position>(e);
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->x, 1);
    EXPECT_EQ(got->y, 2);

    world.remove<Position>(e);
    EXPECT_FALSE(world.has<Position>(e));
    EXPECT_EQ(world.get<Position>(e), nullptr);
    EXPECT_EQ(world.tryGet<Position>(e), nullptr);
}

TEST(WorldComponentsTest, AddTwiceReplacesComponentValue)
{
    World  world;
    Entity e = world.createEntity();

    world.add<Position>(e, 1, 2);
    Position* p = world.add<Position>(e, 9, 10);

    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->x, 9);
    EXPECT_EQ(p->y, 10);
}

TEST(WorldComponentsTest, DestroyEntityRemovesItsComponents)
{
    World  world;
    Entity e1 = world.createEntity();
    const uint32_t idx = e1.index;

    world.add<Position>(e1, 5, 6);
    world.add<Velocity>(e1, 1, 1);

    EXPECT_TRUE(world.has<Position>(e1));
    EXPECT_TRUE(world.has<Velocity>(e1));
    EXPECT_FALSE(world.getComposition(e1).empty());

    world.destroyEntity(e1);
    EXPECT_FALSE(world.isAlive(e1));

    Entity e2 = world.createEntity();
    ASSERT_EQ(e2.index, idx);
    ASSERT_NE(e2.generation, e1.generation);

    EXPECT_FALSE(world.has<Position>(e2));
    EXPECT_FALSE(world.has<Velocity>(e2));
    EXPECT_TRUE(world.getComposition(e2).empty());
}

TEST(WorldComponentsTest, CompositionTracksAddAndRemove)
{
    World  world;
    Entity e = world.createEntity();

    EXPECT_TRUE(world.getComposition(e).empty());

    world.add<Position>(e, 1, 2);
    const auto& afterAdd = world.getComposition(e);
    EXPECT_EQ(std::count(afterAdd.begin(), afterAdd.end(), std::type_index(typeid(Position))), 1);

    world.add<Velocity>(e, 3, 4);
    const auto& afterAdd2 = world.getComposition(e);
    EXPECT_EQ(std::count(afterAdd2.begin(), afterAdd2.end(), std::type_index(typeid(Position))), 1);
    EXPECT_EQ(std::count(afterAdd2.begin(), afterAdd2.end(), std::type_index(typeid(Velocity))), 1);

    world.remove<Position>(e);
    const auto& afterRemove = world.getComposition(e);
    EXPECT_EQ(std::count(afterRemove.begin(), afterRemove.end(), std::type_index(typeid(Position))), 0);
    EXPECT_EQ(std::count(afterRemove.begin(), afterRemove.end(), std::type_index(typeid(Velocity))), 1);
}

TEST(WorldComponentsTest, QueueAddAndFlushAppliesComponent)
{
    World  world;
    Entity e = world.createEntity();

    world.queueAdd<Position>(e, 7, 8);
    EXPECT_FALSE(world.has<Position>(e));

    world.flushCommandBuffer();

    EXPECT_TRUE(world.has<Position>(e));
    const Position* p = world.get<Position>(e);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->x, 7);
    EXPECT_EQ(p->y, 8);
}

TEST(WorldComponentsTest, QueueRemoveAndFlushRemovesComponent)
{
    World  world;
    Entity e = world.createEntity();

    world.add<Position>(e, 1, 2);
    ASSERT_TRUE(world.has<Position>(e));

    world.queueRemove<Position>(e);
    world.flushCommandBuffer();

    EXPECT_FALSE(world.has<Position>(e));
    EXPECT_EQ(world.tryGet<Position>(e), nullptr);
}

TEST(WorldComponentsTest, QueueDestroyAndFlushDestroysEntity)
{
    World  world;
    Entity e = world.createEntity();

    world.queueDestroy(e);
    EXPECT_EQ(world.pendingDestroyCount(), 1u);

    world.flushCommandBuffer();

    EXPECT_EQ(world.pendingDestroyCount(), 0u);
    EXPECT_FALSE(world.isAlive(e));
}

TEST(WorldComponentsTest, QueueAddThenQueueDestroyDoesNotLeakComponentsOnReuse)
{
    World  world;
    Entity e1 = world.createEntity();
    const uint32_t idx = e1.index;

    world.queueAdd<Position>(e1, 1, 2);
    world.queueDestroy(e1);

    world.flushCommandBuffer();

    EXPECT_FALSE(world.isAlive(e1));

    Entity e2 = world.createEntity();
    ASSERT_EQ(e2.index, idx);
    ASSERT_NE(e2.generation, e1.generation);

    EXPECT_FALSE(world.has<Position>(e2));
    EXPECT_EQ(world.tryGet<Position>(e2), nullptr);
    EXPECT_TRUE(world.getComposition(e2).empty());
}

TEST(WorldComponentsTest, ViewVisitsOnlyEntitiesWithAllComponents)
{
    World world;

    Entity a = world.createEntity();
    Entity b = world.createEntity();
    Entity c = world.createEntity();

    world.add<Position>(a, 1, 1);
    world.add<Velocity>(a, 1, 0);

    world.add<Position>(b, 2, 2);

    world.add<Position>(c, 3, 3);
    world.add<Velocity>(c, 0, 1);

    std::vector<Entity> visited;
    world.view<Position, Velocity>(
        [&](Entity e, Position& pos, Velocity& vel)
        {
            (void)pos;
            (void)vel;
            visited.push_back(e);
        });

    ASSERT_EQ(visited.size(), 2u);
    EXPECT_TRUE(std::find(visited.begin(), visited.end(), a) != visited.end());
    EXPECT_TRUE(std::find(visited.begin(), visited.end(), c) != visited.end());
}

TEST(WorldComponentsTest, ViewSortedOrdersEntitiesByIndexEvenAfterSwapAndPop)
{
    World world;

    Entity e1 = world.createEntity();
    Entity e2 = world.createEntity();
    Entity e3 = world.createEntity();

    world.add<Position>(e1, 1, 0);
    world.add<Position>(e2, 2, 0);
    world.add<Position>(e3, 3, 0);

    world.remove<Position>(e2);
    world.add<Position>(e2, 20, 0);

    std::vector<uint32_t> indices;
    world.viewSorted<Position>(
        [&](Entity e, Position& pos)
        {
            (void)pos;
            indices.push_back(e.index);
        });

    ASSERT_EQ(indices.size(), 3u);
    EXPECT_EQ(indices[0], e1.index);
    EXPECT_EQ(indices[1], e2.index);
    EXPECT_EQ(indices[2], e3.index);
}

TEST(WorldComponentsTest, SwapAndPopRemapsSparseIndexCorrectly)
{
    World world;

    Entity a = world.createEntity();
    Entity b = world.createEntity();
    Entity c = world.createEntity();

    world.add<Position>(a, 1, 0);
    world.add<Position>(b, 2, 0);
    world.add<Position>(c, 3, 0);

    world.remove<Position>(b);
    EXPECT_FALSE(world.has<Position>(b));

    const Position* pc1 = world.get<Position>(c);
    ASSERT_NE(pc1, nullptr);
    EXPECT_EQ(pc1->x, 3);

    world.remove<Position>(a);
    EXPECT_FALSE(world.has<Position>(a));

    const Position* pc2 = world.get<Position>(c);
    ASSERT_NE(pc2, nullptr);
    EXPECT_EQ(pc2->x, 3);
}
