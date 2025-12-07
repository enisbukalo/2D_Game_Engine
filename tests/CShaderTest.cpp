#include <gtest/gtest.h>
#include "CShader.h"
#include "Entity.h"
#include "EntityManager.h"
#include "SSerialization.h"

class CShaderTest : public ::testing::Test
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

TEST_F(CShaderTest, ComponentCreationAndDefaults)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>();

    ASSERT_NE(shader, nullptr);
    EXPECT_EQ(shader->getType(), "Shader");
    EXPECT_TRUE(shader->getVertexShaderPath().empty());
    EXPECT_TRUE(shader->getFragmentShaderPath().empty());
    EXPECT_TRUE(shader->isActive());
}

TEST_F(CShaderTest, ParameterizedConstruction)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>("shaders/vertex.glsl", "shaders/fragment.glsl");

    EXPECT_EQ(shader->getVertexShaderPath(), "shaders/vertex.glsl");
    EXPECT_EQ(shader->getFragmentShaderPath(), "shaders/fragment.glsl");
}

TEST_F(CShaderTest, SettersAndGetters)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>();

    shader->setVertexShaderPath("shaders/my_vertex.vert");
    EXPECT_EQ(shader->getVertexShaderPath(), "shaders/my_vertex.vert");

    shader->setFragmentShaderPath("shaders/my_fragment.frag");
    EXPECT_EQ(shader->getFragmentShaderPath(), "shaders/my_fragment.frag");

    shader->setVertexShaderPath("");
    EXPECT_TRUE(shader->getVertexShaderPath().empty());

    shader->setFragmentShaderPath("");
    EXPECT_TRUE(shader->getFragmentShaderPath().empty());
}

TEST_F(CShaderTest, OnlyVertexShader)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>("shaders/vertex.glsl", "");

    EXPECT_EQ(shader->getVertexShaderPath(), "shaders/vertex.glsl");
    EXPECT_TRUE(shader->getFragmentShaderPath().empty());
}

TEST_F(CShaderTest, OnlyFragmentShader)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>("", "shaders/fragment.glsl");

    EXPECT_TRUE(shader->getVertexShaderPath().empty());
    EXPECT_EQ(shader->getFragmentShaderPath(), "shaders/fragment.glsl");
}

TEST_F(CShaderTest, PathsWithSpaces)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>();

    shader->setVertexShaderPath("path with spaces/vertex shader.glsl");
    shader->setFragmentShaderPath("path with spaces/fragment shader.glsl");

    EXPECT_EQ(shader->getVertexShaderPath(), "path with spaces/vertex shader.glsl");
    EXPECT_EQ(shader->getFragmentShaderPath(), "path with spaces/fragment shader.glsl");
}

TEST_F(CShaderTest, Serialization)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>("shaders/test.vert", "shaders/test.frag");

    Serialization::JsonBuilder builder;
    shader->serialize(builder);

    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"cShader\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"vertexShaderPath\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"fragmentShaderPath\"") != std::string::npos);
    EXPECT_TRUE(json.find("shaders/test.vert") != std::string::npos);
    EXPECT_TRUE(json.find("shaders/test.frag") != std::string::npos);
}

TEST_F(CShaderTest, Deserialization)
{
    std::string json = R"({
        "cShader": {
            "vertexShaderPath": "assets/shaders/main.vert",
            "fragmentShaderPath": "assets/shaders/main.frag"
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>();
    shader->deserialize(value);

    EXPECT_EQ(shader->getVertexShaderPath(), "assets/shaders/main.vert");
    EXPECT_EQ(shader->getFragmentShaderPath(), "assets/shaders/main.frag");
}

TEST_F(CShaderTest, SerializeDeserializeRoundTrip)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* shader1 = entity->addComponent<CShader>("path/vertex.glsl", "path/fragment.glsl");

    Serialization::JsonBuilder builder;
    shader1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2  = EntityManager::instance().addEntity("test2");
    auto* shader2 = entity2->addComponent<CShader>();
    shader2->deserialize(value);

    EXPECT_EQ(shader1->getVertexShaderPath(), shader2->getVertexShaderPath());
    EXPECT_EQ(shader1->getFragmentShaderPath(), shader2->getFragmentShaderPath());
}

TEST_F(CShaderTest, EmptyPathsSerialization)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>();

    Serialization::JsonBuilder builder;
    shader->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2  = EntityManager::instance().addEntity("test2");
    auto* shader2 = entity2->addComponent<CShader>();
    shader2->deserialize(value);

    EXPECT_TRUE(shader2->getVertexShaderPath().empty());
    EXPECT_TRUE(shader2->getFragmentShaderPath().empty());
}

TEST_F(CShaderTest, ComponentGuid)
{
    auto entity = EntityManager::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>();

    EXPECT_FALSE(shader->getGuid().empty());

    // Each component should have a unique GUID
    auto  entity2  = EntityManager::instance().addEntity("test2");
    auto* shader2 = entity2->addComponent<CShader>();

    EXPECT_NE(shader->getGuid(), shader2->getGuid());
}

TEST_F(CShaderTest, SerializeWithOnlyVertexShader)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* shader1 = entity->addComponent<CShader>("vertex.glsl", "");

    Serialization::JsonBuilder builder;
    shader1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2  = EntityManager::instance().addEntity("test2");
    auto* shader2 = entity2->addComponent<CShader>();
    shader2->deserialize(value);

    EXPECT_EQ(shader1->getVertexShaderPath(), shader2->getVertexShaderPath());
    EXPECT_EQ(shader1->getFragmentShaderPath(), shader2->getFragmentShaderPath());
    EXPECT_TRUE(shader2->getFragmentShaderPath().empty());
}

TEST_F(CShaderTest, SerializeWithOnlyFragmentShader)
{
    auto entity  = EntityManager::instance().addEntity("test");
    auto* shader1 = entity->addComponent<CShader>("", "fragment.glsl");

    Serialization::JsonBuilder builder;
    shader1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2  = EntityManager::instance().addEntity("test2");
    auto* shader2 = entity2->addComponent<CShader>();
    shader2->deserialize(value);

    EXPECT_EQ(shader1->getVertexShaderPath(), shader2->getVertexShaderPath());
    EXPECT_EQ(shader1->getFragmentShaderPath(), shader2->getFragmentShaderPath());
    EXPECT_TRUE(shader2->getVertexShaderPath().empty());
}
