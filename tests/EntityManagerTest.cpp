#include <gtest/gtest.h>

#include <EntityManager.h>

TEST(EntityManagerTest, CreateReturnsAliveNonNullEntity)
{
    EntityManager em;
    Entity        e = em.create();

    EXPECT_TRUE(e.isValid());
    EXPECT_TRUE(em.isAlive(e));
    EXPECT_NE(e.index, 0u);
}

TEST(EntityManagerTest, IsAliveRejectsNullAndBogusHandles)
{
    EntityManager em;
    EXPECT_FALSE(em.isAlive(Entity::null()));

    // Bogus index should be rejected.
    EXPECT_FALSE(em.isAlive(Entity{999999u, 0u}));

    Entity e = em.create();
    EXPECT_TRUE(em.isAlive(e));

    // Wrong generation should be rejected.
    EXPECT_FALSE(em.isAlive(Entity{e.index, e.generation + 1u}));
}

TEST(EntityManagerTest, DestroyMakesEntityNotAlive)
{
    EntityManager em;
    Entity        e = em.create();

    em.destroy(e);
    EXPECT_FALSE(em.isAlive(e));
}

TEST(EntityManagerTest, DestroyIsIdempotent)
{
    EntityManager em;
    Entity        e = em.create();

    em.destroy(e);
    em.destroy(e);  // should be a no-op
    EXPECT_FALSE(em.isAlive(e));
}

TEST(EntityManagerTest, GenerationBumpsOnReuse)
{
    EntityManager em;

    Entity e1 = em.create();
    const uint32_t idx = e1.index;

    em.destroy(e1);
    EXPECT_FALSE(em.isAlive(e1));

    Entity e2 = em.create();

    EXPECT_EQ(e2.index, idx);
    EXPECT_NE(e2.generation, e1.generation);
    EXPECT_FALSE(em.isAlive(e1));
    EXPECT_TRUE(em.isAlive(e2));
}

TEST(EntityManagerTest, RecyclingIsLifo)
{
    EntityManager em;

    Entity e1 = em.create();
    Entity e2 = em.create();
    Entity e3 = em.create();

    em.destroy(e1);
    em.destroy(e2);

    // LIFO: last destroyed is reused first.
    Entity r1 = em.create();
    EXPECT_EQ(r1.index, e2.index);

    Entity r2 = em.create();
    EXPECT_EQ(r2.index, e1.index);

    // Still alive: e3 was never destroyed.
    EXPECT_TRUE(em.isAlive(e3));
}

TEST(EntityManagerTest, ClearResetsStateAndKeepsNullReserved)
{
    EntityManager em;

    Entity e1 = em.create();
    EXPECT_TRUE(em.isAlive(e1));

    em.clear();

    EXPECT_FALSE(em.isAlive(e1));
    EXPECT_FALSE(em.isAlive(Entity::null()));

    Entity e2 = em.create();
    EXPECT_TRUE(em.isAlive(e2));
    EXPECT_EQ(e2.index, 1u);
}
