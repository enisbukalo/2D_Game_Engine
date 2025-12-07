#include <gtest/gtest.h>
#include "CName.h"
#include "Entity.h"
#include "EntityManager.h"
#include "systems/SSerialization.h"

class CNameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        EntityManager::instance().clear();
    }

    void TearDown() override
    {
        EntityManager::instance().clear();
    }
};

//=============================================================================
// Default Value Tests
//=============================================================================

TEST_F(CNameTest, ComponentCreationAndDefaults)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>();

    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->getType(), "Name");
    EXPECT_TRUE(name->getName().empty());
}

TEST_F(CNameTest, ParameterizedConstruction)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>("Player");

    EXPECT_EQ(name->getName(), "Player");
}

//=============================================================================
// Setter/Getter Tests
//=============================================================================

TEST_F(CNameTest, NameCanBeSet)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>();

    name->setName("Enemy");
    EXPECT_EQ(name->getName(), "Enemy");
}

TEST_F(CNameTest, NameCanBeChanged)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>("Initial");

    EXPECT_EQ(name->getName(), "Initial");

    name->setName("Changed");
    EXPECT_EQ(name->getName(), "Changed");
}

TEST_F(CNameTest, NameCanBeCleared)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>("SomeName");

    name->setName("");
    EXPECT_TRUE(name->getName().empty());
}

TEST_F(CNameTest, NameWithSpaces)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>();

    name->setName("Main Character");
    EXPECT_EQ(name->getName(), "Main Character");
}

TEST_F(CNameTest, NameWithSpecialCharacters)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>();

    name->setName("Player_1: 'Hero'");
    EXPECT_EQ(name->getName(), "Player_1: 'Hero'");
}

TEST_F(CNameTest, NameWithUnicode)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>();

    name->setName("プレイヤー");
    EXPECT_EQ(name->getName(), "プレイヤー");
}

//=============================================================================
// Serialization Tests
//=============================================================================

TEST_F(CNameTest, Serialization)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>("TestEntity");

    JsonBuilder builder;
    name->serialize(builder);
    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"cName\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"name\"") != std::string::npos);
    EXPECT_TRUE(json.find("TestEntity") != std::string::npos);
}

TEST_F(CNameTest, Deserialization)
{
    std::string json = R"({
        "cName": {
            "name": "DeserializedName"
        }
    })";

    JsonValue value(json);

    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>();
    name->deserialize(value);

    EXPECT_EQ(name->getName(), "DeserializedName");
}

TEST_F(CNameTest, SerializeDeserializeRoundTrip)
{
    auto  entity1 = EntityManager::instance().addEntity("test1");
    auto* name1   = entity1->addComponent<CName>("RoundTripTest");

    JsonBuilder builder;
    name1->serialize(builder);
    std::string json = builder.toString();

    JsonValue value(json);

    auto  entity2 = EntityManager::instance().addEntity("test2");
    auto* name2   = entity2->addComponent<CName>();
    name2->deserialize(value);

    EXPECT_EQ(name1->getName(), name2->getName());
}

TEST_F(CNameTest, EmptyNameRoundTrip)
{
    auto  entity1 = EntityManager::instance().addEntity("test1");
    auto* name1   = entity1->addComponent<CName>("");

    JsonBuilder builder;
    name1->serialize(builder);
    std::string json = builder.toString();

    JsonValue value(json);

    auto  entity2 = EntityManager::instance().addEntity("test2");
    auto* name2   = entity2->addComponent<CName>("ShouldBeReplaced");
    name2->deserialize(value);

    EXPECT_TRUE(name2->getName().empty());
}

TEST_F(CNameTest, NameWithSpacesRoundTrip)
{
    auto  entity1 = EntityManager::instance().addEntity("test1");
    auto* name1   = entity1->addComponent<CName>("Name With Spaces");

    JsonBuilder builder;
    name1->serialize(builder);
    std::string json = builder.toString();

    JsonValue value(json);

    auto  entity2 = EntityManager::instance().addEntity("test2");
    auto* name2   = entity2->addComponent<CName>();
    name2->deserialize(value);

    EXPECT_EQ(name2->getName(), "Name With Spaces");
}

TEST_F(CNameTest, LongNameRoundTrip)
{
    std::string longName = "This is a very long name that might be used for testing purposes to ensure that the serialization and deserialization work correctly with longer strings";

    auto  entity1 = EntityManager::instance().addEntity("test1");
    auto* name1   = entity1->addComponent<CName>(longName);

    JsonBuilder builder;
    name1->serialize(builder);
    std::string json = builder.toString();

    JsonValue value(json);

    auto  entity2 = EntityManager::instance().addEntity("test2");
    auto* name2   = entity2->addComponent<CName>();
    name2->deserialize(value);

    EXPECT_EQ(name2->getName(), longName);
}

TEST_F(CNameTest, SpecialCharactersRoundTrip)
{
    auto  entity1 = EntityManager::instance().addEntity("test1");
    auto* name1   = entity1->addComponent<CName>("Test: \"Quotes\" & <Brackets>");

    JsonBuilder builder;
    name1->serialize(builder);
    std::string json = builder.toString();

    JsonValue value(json);

    auto  entity2 = EntityManager::instance().addEntity("test2");
    auto* name2   = entity2->addComponent<CName>();
    name2->deserialize(value);

    EXPECT_EQ(name1->getName(), name2->getName());
}

TEST_F(CNameTest, ComponentGuid)
{
    auto  entity = EntityManager::instance().addEntity("test");
    auto* name   = entity->addComponent<CName>("TestName");

    EXPECT_FALSE(name->getGuid().empty());

    auto  entity2 = EntityManager::instance().addEntity("test2");
    auto* name2   = entity2->addComponent<CName>("TestName2");

    EXPECT_NE(name->getGuid(), name2->getGuid());
}

