#include <gtest/gtest.h>
#include "../include/EntityManager.h"

TEST(EntityManagerTest, EntityCreation)
{
    EntityManager manager;

    std::shared_ptr<Entity> entity = manager.addEntity("test");
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->getTag(), "test");
    EXPECT_TRUE(entity->isAlive());
}

TEST(EntityManagerTest, EntityRemoval)
{
    EntityManager           manager;
    std::shared_ptr<Entity> entity = manager.addEntity("test");

    manager.update(0.0f);  // Process pending entities
    EXPECT_EQ(manager.getEntities().size(), 1);

    manager.removeEntity(entity);
    manager.update(0.0f);  // Process removals
    EXPECT_EQ(manager.getEntities().size(), 0);
}

TEST(EntityManagerTest, EntityTagging)
{
    EntityManager manager;

    manager.addEntity("typeA");
    manager.addEntity("typeA");
    manager.addEntity("typeB");

    manager.update(0.0f);  // Process pending entities

    std::vector<std::shared_ptr<Entity>> typeAEntities = manager.getEntitiesByTag("typeA");
    std::vector<std::shared_ptr<Entity>> typeBEntities = manager.getEntitiesByTag("typeB");

    EXPECT_EQ(typeAEntities.size(), 2);
    EXPECT_EQ(typeBEntities.size(), 1);
}

TEST(EntityManagerTest, EntityComponentQuery)
{
    EntityManager manager;

    std::shared_ptr<Entity> entity1 = manager.addEntity("test1");
    entity1->addComponent<CTransform>();

    std::shared_ptr<Entity> entity2 = manager.addEntity("test2");
    entity2->addComponent<CTransform>();
    entity2->addComponent<CGravity>();

    manager.update(0.0f);  // Process pending entities

    auto entitiesWithTransform = manager.getEntitiesWithComponent<CTransform>();
    auto entitiesWithGravity   = manager.getEntitiesWithComponent<CGravity>();

    EXPECT_EQ(entitiesWithTransform.size(), 2);
    EXPECT_EQ(entitiesWithGravity.size(), 1);
}

TEST(EntityManagerTest, EntityUpdateSystem)
{
    EntityManager manager;

    std::shared_ptr<Entity> entity    = manager.addEntity("test");
    CTransform             *transform = entity->addComponent<CTransform>();
    CGravity               *gravity   = entity->addComponent<CGravity>();

    manager.update(0.0f);  // Process pending entities
    manager.update(1.0f);  // Update components

    EXPECT_FLOAT_EQ(transform->velocity.y, -9.81f);
}