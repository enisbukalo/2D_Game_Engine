#include <gtest/gtest.h>

#include <World.h>

#include <components/CNativeScript.h>
#include <components/CPhysicsBody2D.h>
#include <components/CTransform.h>
#include <systems/S2DPhysics.h>
#include <systems/SScript.h>

namespace
{
class SpawnManyScriptsA final : public Components::INativeScript
{
public:
    SpawnManyScriptsA(int* createCountA, int* updateCountA, int* createCountB, int* updateCountB, int spawnCount)
        : m_createCountA(createCountA)
        , m_updateCountA(updateCountA)
        , m_createCountB(createCountB)
        , m_updateCountB(updateCountB)
        , m_spawnCount(spawnCount)
    {
    }

    void onCreate(Entity /*self*/, World& /*world*/) override
    {
        if (m_createCountA)
        {
            ++(*m_createCountA);
        }
    }

    void onUpdate(float /*deltaTime*/, Entity /*self*/, World& world) override
    {
        if (m_updateCountA)
        {
            ++(*m_updateCountA);
        }

        if (m_spawned)
        {
            return;
        }
        m_spawned = true;

        for (int i = 0; i < m_spawnCount; ++i)
        {
            Entity e = world.createEntity();
            auto* s  = world.add<Components::CNativeScript>(e);
            ASSERT_NE(s, nullptr);
            s->bind<SpawnManyScriptsB>(m_createCountB, m_updateCountB);
        }
    }

private:
    class SpawnManyScriptsB final : public Components::INativeScript
    {
    public:
        SpawnManyScriptsB(int* createCountB, int* updateCountB) : m_createCountB(createCountB), m_updateCountB(updateCountB) {}

        void onCreate(Entity /*self*/, World& /*world*/) override
        {
            if (m_createCountB)
            {
                ++(*m_createCountB);
            }
        }

        void onUpdate(float /*deltaTime*/, Entity /*self*/, World& /*world*/) override
        {
            if (m_updateCountB)
            {
                ++(*m_updateCountB);
            }
        }

    private:
        int* m_createCountB{nullptr};
        int* m_updateCountB{nullptr};
    };

    int* m_createCountA{nullptr};
    int* m_updateCountA{nullptr};
    int* m_createCountB{nullptr};
    int* m_updateCountB{nullptr};

    int  m_spawnCount{0};
    bool m_spawned{false};
};
}

TEST(SScriptRegressionTest, SpawningEntitiesWithScriptsDuringUpdateIsSafeAndDeferredToNextTick)
{
    World world;

    int createA = 0;
    int updateA = 0;
    int createB = 0;
    int updateB = 0;

    constexpr int kSpawnCount = 50;

    Entity a = world.createEntity();
    auto*  s = world.add<Components::CNativeScript>(a);
    ASSERT_NE(s, nullptr);
    s->bind<SpawnManyScriptsA>(&createA, &updateA, &createB, &updateB, kSpawnCount);

    Systems::SScript scripts;

    scripts.update(1.0f / 60.0f, world);

    EXPECT_EQ(createA, 1);
    EXPECT_EQ(updateA, 1);
    // Newly spawned scripts should not be created/updated in the same tick due to snapshot semantics.
    EXPECT_EQ(createB, 0);
    EXPECT_EQ(updateB, 0);

    scripts.update(1.0f / 60.0f, world);

    EXPECT_EQ(createA, 1);
    EXPECT_EQ(updateA, 2);
    EXPECT_EQ(createB, kSpawnCount);
    EXPECT_EQ(updateB, kSpawnCount);
}

TEST(S2DPhysicsRegressionTest, FixedUpdateCallbackRegisteredBeforeBodyExistsRunsOnceBodyIsCreated)
{
    World world;

    Entity e = world.createEntity();
    world.add<Components::CTransform>(e);
    world.add<Components::CPhysicsBody2D>(e);

    Systems::S2DPhysics physics;

    int callbackCalls = 0;
    physics.setFixedUpdateCallback(e, [&](float /*dt*/) { ++callbackCalls; });

    // First fixed step: callback runs before body creation, so it should be skipped but retained.
    physics.fixedUpdate(physics.getTimeStep(), world);
    EXPECT_EQ(callbackCalls, 0);

    // Second fixed step: body exists (created during previous update), so callback should run.
    physics.fixedUpdate(physics.getTimeStep(), world);
    EXPECT_EQ(callbackCalls, 1);
}
