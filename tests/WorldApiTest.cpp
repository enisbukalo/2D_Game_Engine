#include <gtest/gtest.h>

#include <World.h>

namespace
{
struct TestComponent
{
    int value = 0;

    TestComponent() = default;
    explicit TestComponent(int v) : value(v) {}
};
}

TEST(WorldApiTest, CreateEntityIsAliveAndNonNull)
{
    World  world;
    Entity e = world.createEntity();

    EXPECT_TRUE(e.isValid());
    EXPECT_TRUE(world.isAlive(e));
}

TEST(WorldApiTest, DestroyEntityInvalidatesHandle)
{
    World  world;
    Entity e = world.createEntity();

    world.destroyEntity(e);
    EXPECT_FALSE(world.isAlive(e));
}

TEST(WorldApiTest, GenerationBumpsOnReuseThroughWorld)
{
    World  world;
    Entity e1 = world.createEntity();
    const uint32_t idx = e1.index;

    world.destroyEntity(e1);
    EXPECT_FALSE(world.isAlive(e1));

    Entity e2 = world.createEntity();

    EXPECT_EQ(e2.index, idx);
    EXPECT_NE(e2.generation, e1.generation);
    EXPECT_FALSE(world.isAlive(e1));
    EXPECT_TRUE(world.isAlive(e2));
}

#if !defined(NDEBUG)
TEST(WorldApiDeathTest, NullEntityMutationsAssert)
{
    EXPECT_DEATH(
        {
            World world;
            world.destroyEntity(Entity::null());
        },
        "null entity");

    EXPECT_DEATH(
        {
            World world;
            world.add<TestComponent>(Entity::null(), 123);
        },
        "null entity");
}

TEST(WorldApiDeathTest, DeadEntityMutationsAssert)
{
    EXPECT_DEATH(
        {
            World  world;
            Entity e = world.createEntity();
            world.destroyEntity(e);
            world.destroyEntity(e);
        },
        "dead entity");

    EXPECT_DEATH(
        {
            World  world;
            Entity e = world.createEntity();
            world.destroyEntity(e);
            world.add<TestComponent>(e, 123);
        },
        "dead entity");
}
#endif
