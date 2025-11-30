#include <gtest/gtest.h>
#include "CTexture.h"
#include "Entity.h"
#include "EntityManager.h"
#include "JsonBuilder.h"
#include "JsonValue.h"

class CTextureTest : public ::testing::Test
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

TEST_F(CTextureTest, ComponentCreationAndDefaults)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>();

    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->getType(), "Texture");
    EXPECT_TRUE(texture->getTexturePath().empty());
    EXPECT_TRUE(texture->isActive());
}

TEST_F(CTextureTest, ParameterizedConstruction)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>("assets/textures/sprite.png");

    EXPECT_EQ(texture->getTexturePath(), "assets/textures/sprite.png");
}

TEST_F(CTextureTest, SettersAndGetters)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>();

    texture->setTexturePath("path/to/texture.png");
    EXPECT_EQ(texture->getTexturePath(), "path/to/texture.png");

    texture->setTexturePath("another/texture.jpg");
    EXPECT_EQ(texture->getTexturePath(), "another/texture.jpg");

    texture->setTexturePath("");
    EXPECT_TRUE(texture->getTexturePath().empty());
}

TEST_F(CTextureTest, PathWithSpaces)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>();

    texture->setTexturePath("path with spaces/my texture.png");
    EXPECT_EQ(texture->getTexturePath(), "path with spaces/my texture.png");
}

TEST_F(CTextureTest, PathWithSpecialCharacters)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>();

    texture->setTexturePath("path/with-special_chars.123/texture.png");
    EXPECT_EQ(texture->getTexturePath(), "path/with-special_chars.123/texture.png");
}

TEST_F(CTextureTest, Serialization)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>("assets/player_sprite.png");

    JsonBuilder builder;
    texture->serialize(builder);

    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"cTexture\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"texturePath\"") != std::string::npos);
    EXPECT_TRUE(json.find("assets/player_sprite.png") != std::string::npos);
}

TEST_F(CTextureTest, Deserialization)
{
    std::string json = R"({
        "cTexture": {
            "texturePath": "assets/enemy_sprite.png"
        }
    })";

    JsonValue value(json);

    auto entity = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>();
    texture->deserialize(value);

    EXPECT_EQ(texture->getTexturePath(), "assets/enemy_sprite.png");
}

TEST_F(CTextureTest, SerializeDeserializeRoundTrip)
{
    auto entity   = EntityManager::instance().addEntity("test");
    auto* texture1 = entity->addComponent<CTexture>("path/to/my/texture.png");

    JsonBuilder builder;
    texture1->serialize(builder);
    std::string json = builder.toString();

    JsonValue value(json);

    auto  entity2  = EntityManager::instance().addEntity("test2");
    auto* texture2 = entity2->addComponent<CTexture>();
    texture2->deserialize(value);

    EXPECT_EQ(texture1->getTexturePath(), texture2->getTexturePath());
}

TEST_F(CTextureTest, EmptyPathSerialization)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>();

    JsonBuilder builder;
    texture->serialize(builder);
    std::string json = builder.toString();

    JsonValue value(json);

    auto  entity2  = EntityManager::instance().addEntity("test2");
    auto* texture2 = entity2->addComponent<CTexture>();
    texture2->deserialize(value);

    EXPECT_TRUE(texture2->getTexturePath().empty());
}

TEST_F(CTextureTest, ComponentGuid)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>();

    EXPECT_FALSE(texture->getGuid().empty());

    // Each component should have a unique GUID
    auto  entity2  = EntityManager::instance().addEntity("test2");
    auto* texture2 = entity2->addComponent<CTexture>();

    EXPECT_NE(texture->getGuid(), texture2->getGuid());
}
