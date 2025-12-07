#include <gtest/gtest.h>
#include "CRenderable.h"
#include "Entity.h"
#include "SEntity.h"
#include "SSerialization.h"

class CRenderableTest : public ::testing::Test
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

TEST_F(CRenderableTest, ComponentCreationAndDefaults)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>();

    ASSERT_NE(renderable, nullptr);
    EXPECT_EQ(renderable->getType(), "Renderable");
    EXPECT_EQ(renderable->getVisualType(), VisualType::None);
    EXPECT_EQ(renderable->getColor(), Color::White);
    EXPECT_EQ(renderable->getZIndex(), 0);
    EXPECT_TRUE(renderable->isVisible());
    EXPECT_TRUE(renderable->isActive());
}

TEST_F(CRenderableTest, ParameterizedConstruction)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>(VisualType::Circle, Color::Red, 5, false);

    EXPECT_EQ(renderable->getVisualType(), VisualType::Circle);
    EXPECT_EQ(renderable->getColor(), Color::Red);
    EXPECT_EQ(renderable->getZIndex(), 5);
    EXPECT_FALSE(renderable->isVisible());
}

TEST_F(CRenderableTest, SettersAndGetters)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>();

    renderable->setVisualType(VisualType::Rectangle);
    EXPECT_EQ(renderable->getVisualType(), VisualType::Rectangle);

    Color testColor(128, 64, 32, 200);
    renderable->setColor(testColor);
    EXPECT_EQ(renderable->getColor(), testColor);

    renderable->setZIndex(10);
    EXPECT_EQ(renderable->getZIndex(), 10);

    renderable->setVisible(false);
    EXPECT_FALSE(renderable->isVisible());

    renderable->setVisible(true);
    EXPECT_TRUE(renderable->isVisible());
}

TEST_F(CRenderableTest, VisualTypeEnumeration)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>();

    // Test all visual types
    renderable->setVisualType(VisualType::None);
    EXPECT_EQ(renderable->getVisualType(), VisualType::None);

    renderable->setVisualType(VisualType::Rectangle);
    EXPECT_EQ(renderable->getVisualType(), VisualType::Rectangle);

    renderable->setVisualType(VisualType::Circle);
    EXPECT_EQ(renderable->getVisualType(), VisualType::Circle);

    renderable->setVisualType(VisualType::Sprite);
    EXPECT_EQ(renderable->getVisualType(), VisualType::Sprite);

    renderable->setVisualType(VisualType::Custom);
    EXPECT_EQ(renderable->getVisualType(), VisualType::Custom);
}

TEST_F(CRenderableTest, ZIndexOrdering)
{
    auto entity1 = SEntity::instance().addEntity("entity1");
    auto entity2 = SEntity::instance().addEntity("entity2");
    auto entity3 = SEntity::instance().addEntity("entity3");

    auto* r1 = entity1->addComponent<CRenderable>(VisualType::Rectangle, Color::White, -5);
    auto* r2 = entity2->addComponent<CRenderable>(VisualType::Rectangle, Color::White, 0);
    auto* r3 = entity3->addComponent<CRenderable>(VisualType::Rectangle, Color::White, 10);

    EXPECT_LT(r1->getZIndex(), r2->getZIndex());
    EXPECT_LT(r2->getZIndex(), r3->getZIndex());
}

TEST_F(CRenderableTest, Serialization)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>(VisualType::Sprite, Color(255, 128, 64, 200), 7, false);

    Serialization::JsonBuilder builder;
    renderable->serialize(builder);

    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"cRenderable\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"visualType\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"color\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"zIndex\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"visible\"") != std::string::npos);
}

TEST_F(CRenderableTest, Deserialization)
{
    std::string json = R"({
        "cRenderable": {
            "visualType": 2,
            "color": {
                "r": 100,
                "g": 150,
                "b": 200,
                "a": 128
            },
            "zIndex": 5,
            "visible": false
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto entity = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>();
    renderable->deserialize(value);

    EXPECT_EQ(renderable->getVisualType(), VisualType::Circle);
    EXPECT_EQ(renderable->getColor().r, 100);
    EXPECT_EQ(renderable->getColor().g, 150);
    EXPECT_EQ(renderable->getColor().b, 200);
    EXPECT_EQ(renderable->getColor().a, 128);
    EXPECT_EQ(renderable->getZIndex(), 5);
    EXPECT_FALSE(renderable->isVisible());
}

TEST_F(CRenderableTest, SerializeDeserializeRoundTrip)
{
    auto entity      = SEntity::instance().addEntity("test");
    auto* renderable1 = entity->addComponent<CRenderable>(VisualType::Rectangle, Color(64, 128, 192, 255), -3, true);

    Serialization::JsonBuilder builder;
    renderable1->serialize(builder);
    std::string json = builder.toString();    Serialization::SSerialization::JsonValue value(json);

    auto  entity2  = SEntity::instance().addEntity("test2");
    auto* renderable2 = entity2->addComponent<CRenderable>();
    renderable2->deserialize(value);

    EXPECT_EQ(renderable1->getVisualType(), renderable2->getVisualType());
    EXPECT_EQ(renderable1->getColor(), renderable2->getColor());
    EXPECT_EQ(renderable1->getZIndex(), renderable2->getZIndex());
    EXPECT_EQ(renderable1->isVisible(), renderable2->isVisible());
}

TEST_F(CRenderableTest, VisibilityToggle)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>();

    EXPECT_TRUE(renderable->isVisible());

    renderable->setVisible(false);
    EXPECT_FALSE(renderable->isVisible());

    renderable->setVisible(true);
    EXPECT_TRUE(renderable->isVisible());
}

TEST_F(CRenderableTest, ColorEquality)
{
    Color c1(255, 128, 64, 200);
    Color c2(255, 128, 64, 200);
    Color c3(255, 128, 64, 100);

    EXPECT_EQ(c1, c2);
    EXPECT_NE(c1, c3);
}

TEST_F(CRenderableTest, LineVisualType)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>();

    renderable->setVisualType(VisualType::Line);
    EXPECT_EQ(renderable->getVisualType(), VisualType::Line);
}

TEST_F(CRenderableTest, LineEndpoints)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>(VisualType::Line);

    // Test default endpoints
    Vec2 start = renderable->getLineStart();
    Vec2 end   = renderable->getLineEnd();
    EXPECT_FLOAT_EQ(start.x, 0.0f);
    EXPECT_FLOAT_EQ(start.y, 0.0f);
    EXPECT_FLOAT_EQ(end.x, 1.0f);
    EXPECT_FLOAT_EQ(end.y, 0.0f);

    // Test setting endpoints
    renderable->setLineStart(Vec2(1.0f, 2.0f));
    renderable->setLineEnd(Vec2(3.0f, 4.0f));

    start = renderable->getLineStart();
    end   = renderable->getLineEnd();
    EXPECT_FLOAT_EQ(start.x, 1.0f);
    EXPECT_FLOAT_EQ(start.y, 2.0f);
    EXPECT_FLOAT_EQ(end.x, 3.0f);
    EXPECT_FLOAT_EQ(end.y, 4.0f);
}

TEST_F(CRenderableTest, LineThickness)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>(VisualType::Line);

    // Test default thickness
    EXPECT_FLOAT_EQ(renderable->getLineThickness(), 2.0f);

    // Test setting thickness
    renderable->setLineThickness(5.5f);
    EXPECT_FLOAT_EQ(renderable->getLineThickness(), 5.5f);

    renderable->setLineThickness(1.0f);
    EXPECT_FLOAT_EQ(renderable->getLineThickness(), 1.0f);
}

TEST_F(CRenderableTest, LineSerializationDeserialization)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>(VisualType::Line, Color::Yellow, 3, true);

    renderable->setLineStart(Vec2(-1.5f, 2.5f));
    renderable->setLineEnd(Vec2(3.5f, -4.5f));
    renderable->setLineThickness(3.5f);

    Serialization::JsonBuilder builder;
    renderable->serialize(builder);
    std::string json = builder.toString();

    // Verify line properties are serialized
    EXPECT_TRUE(json.find("\"lineStart\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"lineEnd\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"lineThickness\"") != std::string::npos);

    // Deserialize and verify
    Serialization::SSerialization::JsonValue value(json);
    auto  entity2  = SEntity::instance().addEntity("test2");
    auto* renderable2 = entity2->addComponent<CRenderable>();
    renderable2->deserialize(value);

    EXPECT_EQ(renderable2->getVisualType(), VisualType::Line);
    EXPECT_FLOAT_EQ(renderable2->getLineStart().x, -1.5f);
    EXPECT_FLOAT_EQ(renderable2->getLineStart().y, 2.5f);
    EXPECT_FLOAT_EQ(renderable2->getLineEnd().x, 3.5f);
    EXPECT_FLOAT_EQ(renderable2->getLineEnd().y, -4.5f);
    EXPECT_FLOAT_EQ(renderable2->getLineThickness(), 3.5f);
}

TEST_F(CRenderableTest, NonLineTypeDoesNotSerializeLineProperties)
{
    auto entity     = SEntity::instance().addEntity("test");
    auto* renderable = entity->addComponent<CRenderable>(VisualType::Rectangle);

    // Set line properties (should be ignored for non-Line types)
    renderable->setLineStart(Vec2(1.0f, 2.0f));
    renderable->setLineEnd(Vec2(3.0f, 4.0f));
    renderable->setLineThickness(5.0f);

    Serialization::JsonBuilder builder;
    renderable->serialize(builder);
    std::string json = builder.toString();

    // Verify line properties are NOT serialized for non-Line types
    EXPECT_TRUE(json.find("\"lineStart\"") == std::string::npos);
    EXPECT_TRUE(json.find("\"lineEnd\"") == std::string::npos);
    EXPECT_TRUE(json.find("\"lineThickness\"") == std::string::npos);
}
