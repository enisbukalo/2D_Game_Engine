#include <gtest/gtest.h>
#include "CMaterial.h"
#include "CTexture.h"
#include "CShader.h"
#include "Entity.h"
#include "SEntity.h"
#include "SSerialization.h"

class CMaterialTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SEntity::instance().clear();
    }

    void TearDown() override
    {
        SEntity::instance().clear();
    }
};

TEST_F(CMaterialTest, ComponentCreationAndDefaults)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();

    ASSERT_NE(material, nullptr);
    EXPECT_EQ(material->getType(), "Material");
    EXPECT_TRUE(material->getTextureGuid().empty());
    EXPECT_TRUE(material->getShaderGuid().empty());
    EXPECT_EQ(material->getTint(), Color::White);
    EXPECT_EQ(material->getBlendMode(), BlendMode::Alpha);
    EXPECT_FLOAT_EQ(material->getOpacity(), 1.0f);
    EXPECT_TRUE(material->isActive());
}

TEST_F(CMaterialTest, ParameterizedConstruction)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>(
        "texture-guid-123",
        "shader-guid-456",
        Color::Red,
        BlendMode::Add,
        0.75f
    );

    EXPECT_EQ(material->getTextureGuid(), "texture-guid-123");
    EXPECT_EQ(material->getShaderGuid(), "shader-guid-456");
    EXPECT_EQ(material->getTint(), Color::Red);
    EXPECT_EQ(material->getBlendMode(), BlendMode::Add);
    EXPECT_FLOAT_EQ(material->getOpacity(), 0.75f);
}

TEST_F(CMaterialTest, SettersAndGetters)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();

    material->setTextureGuid("texture-abc");
    EXPECT_EQ(material->getTextureGuid(), "texture-abc");

    material->setShaderGuid("shader-xyz");
    EXPECT_EQ(material->getShaderGuid(), "shader-xyz");

    material->setTint(Color::Blue);
    EXPECT_EQ(material->getTint(), Color::Blue);

    material->setBlendMode(BlendMode::Multiply);
    EXPECT_EQ(material->getBlendMode(), BlendMode::Multiply);

    material->setOpacity(0.5f);
    EXPECT_FLOAT_EQ(material->getOpacity(), 0.5f);
}

TEST_F(CMaterialTest, BlendModeEnumeration)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();

    material->setBlendMode(BlendMode::Alpha);
    EXPECT_EQ(material->getBlendMode(), BlendMode::Alpha);

    material->setBlendMode(BlendMode::Add);
    EXPECT_EQ(material->getBlendMode(), BlendMode::Add);

    material->setBlendMode(BlendMode::Multiply);
    EXPECT_EQ(material->getBlendMode(), BlendMode::Multiply);

    material->setBlendMode(BlendMode::None);
    EXPECT_EQ(material->getBlendMode(), BlendMode::None);
}

TEST_F(CMaterialTest, OpacityBounds)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();

    material->setOpacity(0.0f);
    EXPECT_FLOAT_EQ(material->getOpacity(), 0.0f);

    material->setOpacity(1.0f);
    EXPECT_FLOAT_EQ(material->getOpacity(), 1.0f);

    // Values outside [0, 1] should still be accepted (clamping is renderer's responsibility)
    material->setOpacity(-0.5f);
    EXPECT_FLOAT_EQ(material->getOpacity(), -0.5f);

    material->setOpacity(2.0f);
    EXPECT_FLOAT_EQ(material->getOpacity(), 2.0f);
}

TEST_F(CMaterialTest, IntegrationWithTextureComponent)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* texture = entity->addComponent<CTexture>("assets/sprite.png");
    auto* material = entity->addComponent<CMaterial>();

    material->setTextureGuid(texture->getGuid());
    EXPECT_EQ(material->getTextureGuid(), texture->getGuid());
}

TEST_F(CMaterialTest, IntegrationWithShaderComponent)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* shader = entity->addComponent<CShader>("vertex.glsl", "fragment.glsl");
    auto* material = entity->addComponent<CMaterial>();

    material->setShaderGuid(shader->getGuid());
    EXPECT_EQ(material->getShaderGuid(), shader->getGuid());
}

TEST_F(CMaterialTest, Serialization)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>(
        "texture-guid-001",
        "shader-guid-002",
        Color::Green,
        BlendMode::Add,
        0.8f
    );

    Serialization::JsonBuilder builder;
    material->serialize(builder);
    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"cMaterial\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"textureGuid\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"shaderGuid\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"tint\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"blendMode\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"opacity\"") != std::string::npos);
    EXPECT_TRUE(json.find("texture-guid-001") != std::string::npos);
    EXPECT_TRUE(json.find("shader-guid-002") != std::string::npos);
}

TEST_F(CMaterialTest, Deserialization)
{
    std::string json = R"({
        "cMaterial": {
            "textureGuid": "my-texture-guid",
            "shaderGuid": "my-shader-guid",
            "tint": {
                "r": 255,
                "g": 0,
                "b": 255,
                "a": 200
            },
            "blendMode": 2,
            "opacity": 0.6
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();
    material->deserialize(value);

    EXPECT_EQ(material->getTextureGuid(), "my-texture-guid");
    EXPECT_EQ(material->getShaderGuid(), "my-shader-guid");
    EXPECT_EQ(material->getTint().r, 255);
    EXPECT_EQ(material->getTint().g, 0);
    EXPECT_EQ(material->getTint().b, 255);
    EXPECT_EQ(material->getTint().a, 200);
    EXPECT_EQ(material->getBlendMode(), BlendMode::Multiply);
    EXPECT_FLOAT_EQ(material->getOpacity(), 0.6f);
}

TEST_F(CMaterialTest, SerializeDeserializeRoundTrip)
{
    auto entity1 = SEntity::instance().addEntity("test1");
    auto* material1 = entity1->addComponent<CMaterial>(
        "tex-123",
        "shader-456",
        Color(128, 64, 200, 255),
        BlendMode::Add,
        0.45f
    );

    Serialization::JsonBuilder builder;
    material1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto entity2 = SEntity::instance().addEntity("test2");
    auto* material2 = entity2->addComponent<CMaterial>();
    material2->deserialize(value);

    EXPECT_EQ(material1->getTextureGuid(), material2->getTextureGuid());
    EXPECT_EQ(material1->getShaderGuid(), material2->getShaderGuid());
    EXPECT_EQ(material1->getTint(), material2->getTint());
    EXPECT_EQ(material1->getBlendMode(), material2->getBlendMode());
    EXPECT_FLOAT_EQ(material1->getOpacity(), material2->getOpacity());
}

TEST_F(CMaterialTest, EmptyGuidsSerializtion)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();

    Serialization::JsonBuilder builder;
    material->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto entity2 = SEntity::instance().addEntity("test2");
    auto* material2 = entity2->addComponent<CMaterial>();
    material2->deserialize(value);

    EXPECT_TRUE(material2->getTextureGuid().empty());
    EXPECT_TRUE(material2->getShaderGuid().empty());
}

TEST_F(CMaterialTest, ComponentGuid)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();

    EXPECT_FALSE(material->getGuid().empty());

    auto entity2 = SEntity::instance().addEntity("test2");
    auto* material2 = entity2->addComponent<CMaterial>();

    EXPECT_NE(material->getGuid(), material2->getGuid());
}

TEST_F(CMaterialTest, BlendModeStringConversion)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();

    // Test each blend mode through serialization
    BlendMode modes[] = {
        BlendMode::Alpha,
        BlendMode::Add,
        BlendMode::Multiply,
        BlendMode::None
    };

    for (auto mode : modes)
    {
        material->setBlendMode(mode);

        Serialization::JsonBuilder builder;
        material->serialize(builder);
        std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

        auto entity2 = SEntity::instance().addEntity("test_mode");
        auto* material2 = entity2->addComponent<CMaterial>();
        material2->deserialize(value);

        EXPECT_EQ(material->getBlendMode(), material2->getBlendMode());

        SEntity::instance().removeEntity(entity2);
    }
}

TEST_F(CMaterialTest, TintColorTransparency)
{
    auto entity = SEntity::instance().addEntity("test");
    auto* material = entity->addComponent<CMaterial>();

    // Test fully transparent tint
    Color transparent(255, 255, 255, 0);
    material->setTint(transparent);
    EXPECT_EQ(material->getTint().a, 0);

    // Test semi-transparent tint
    Color semiTransparent(255, 255, 255, 128);
    material->setTint(semiTransparent);
    EXPECT_EQ(material->getTint().a, 128);

    // Test fully opaque tint
    Color opaque(255, 255, 255, 255);
    material->setTint(opaque);
    EXPECT_EQ(material->getTint().a, 255);
}
