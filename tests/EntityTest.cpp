#include <gtest/gtest.h>

#include <Entity.h>

#include <unordered_set>

TEST(EntityTest, NullEntityIsInvalid)
{
    Entity n = Entity::null();
    EXPECT_FALSE(n.isValid());
    EXPECT_FALSE(static_cast<bool>(n));
    EXPECT_EQ(n.index, 0u);
    EXPECT_EQ(n.generation, 0u);
}

TEST(EntityTest, ValidEntityIsTruthy)
{
    Entity e{1u, 0u};
    EXPECT_TRUE(e.isValid());
    EXPECT_TRUE(static_cast<bool>(e));
}

TEST(EntityTest, EqualityAndOrderingCompareIndexAndGeneration)
{
    Entity a{1u, 0u};
    Entity b{1u, 1u};
    Entity c{2u, 0u};

    EXPECT_NE(a, b);
    EXPECT_LT(a, b);
    EXPECT_LT(b, c);
}

TEST(EntityTest, HashWorksInUnorderedSet)
{
    std::unordered_set<Entity> set;

    Entity a{1u, 0u};
    Entity b{1u, 1u};

    set.insert(a);
    EXPECT_EQ(set.count(a), 1u);
    EXPECT_EQ(set.count(b), 0u);

    set.insert(b);
    EXPECT_EQ(set.count(b), 1u);
}
