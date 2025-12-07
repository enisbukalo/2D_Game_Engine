#include <gtest/gtest.h>
#include <filesystem>
#include "Entity.h"
#include "SEntity.h"
#include "ComponentFactory.h"
#include "CName.h"
#include "CTransform.h"
#include "CPhysicsBody2D.h"
#include "CCollider2D.h"
#include "CRenderable.h"
#include "CTexture.h"
#include "CShader.h"
#include "CMaterial.h"
#include "CInputController.h"
#include "CParticleEmitter.h"
#include "CAudioSource.h"
#include "CAudioListener.h"
#include "SSerialization.h"
#include "SInput.h"
#include "S2DPhysics.h"
#include "SAudio.h"
#include "TestUtils.h"

// Test fixture for entity tests
class EntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Clear the EntityManager before each test
        SEntity::instance().clear();
        // Initialize SInputManager for components that rely on it
        SInput::instance().shutdown();
        SInput::instance().initialize(nullptr, false);
        // Initialize audio system
        SAudio::instance().initialize();
    }

    void TearDown() override
    {
        // Shutdown systems after tests
        SInput::instance().shutdown();
        SAudio::instance().shutdown();
    }
};

// =============================================================================
// Basic Entity Tests
// =============================================================================

TEST_F(EntityTest, EntityCreation)
{
    auto entity = SEntity::instance().addEntity("test");
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->getTag(), "test");
    EXPECT_TRUE(entity->isAlive());
    EXPECT_FALSE(entity->getGuid().empty());
}

TEST_F(EntityTest, EntityDestroy)
{
    auto entity = SEntity::instance().addEntity("test");
    EXPECT_TRUE(entity->isAlive());
    entity->destroy();
    EXPECT_FALSE(entity->isAlive());
}

TEST_F(EntityTest, EntityAddComponent)
{
    auto entity = SEntity::instance().addEntity("test");
    auto transform = entity->addComponent<CTransform>();
    EXPECT_NE(transform, nullptr);
    EXPECT_EQ(entity->getComponent<CTransform>(), transform);
}

TEST_F(EntityTest, EntityHasComponent)
{
    auto entity = SEntity::instance().addEntity("test");
    EXPECT_FALSE(entity->hasComponent<CTransform>());
    entity->addComponent<CTransform>();
    EXPECT_TRUE(entity->hasComponent<CTransform>());
}

TEST_F(EntityTest, EntityRemoveComponent)
{
    auto entity = SEntity::instance().addEntity("test");
    entity->addComponent<CTransform>();
    EXPECT_TRUE(entity->hasComponent<CTransform>());
    entity->removeComponent<CTransform>();
    EXPECT_FALSE(entity->hasComponent<CTransform>());
}

// =============================================================================
// Entity Serialization Tests
// =============================================================================

TEST_F(EntityTest, SerializeBasicEntity)
{
    auto entity = SEntity::instance().addEntity("test_entity");
    auto transform = entity->addComponent<CTransform>();
    transform->setPosition(Vec2(100.0f, 200.0f));
    
    Serialization::JsonBuilder builder;
    entity->serialize(builder);
    std::string json = builder.toString();
    
    EXPECT_TRUE(json.find("test_entity") != std::string::npos);
    EXPECT_TRUE(json.find("cTransform") != std::string::npos);
    EXPECT_TRUE(json.find("100") != std::string::npos);
    EXPECT_TRUE(json.find("200") != std::string::npos);
}

TEST_F(EntityTest, SerializeEntityWithMultipleComponents)
{
    auto entity = SEntity::instance().addEntity("multi_component");
    entity->addComponent<CTransform>()->setPosition(Vec2(50.0f, 75.0f));
    entity->addComponent<CName>()->setName("TestName");
    entity->addComponent<CRenderable>();
    
    Serialization::JsonBuilder builder;
    entity->serialize(builder);
    std::string json = builder.toString();
    
    EXPECT_TRUE(json.find("cTransform") != std::string::npos);
    EXPECT_TRUE(json.find("cName") != std::string::npos);
    EXPECT_TRUE(json.find("cRenderable") != std::string::npos);
    EXPECT_TRUE(json.find("TestName") != std::string::npos);
}

// =============================================================================
// Entity Deserialization - Component Type Mapping Tests
// =============================================================================

TEST_F(EntityTest, DeserializeTransformComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cTransform": {
                    "position": {"x": 100.0, "y": 200.0},
                    "velocity": {"x": 5.0, "y": -3.0},
                    "scale": {"x": 2.0, "y": 2.0},
                    "rotation": 45.0
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto transform = entity->getComponent<CTransform>();
    ASSERT_NE(transform, nullptr);
    EXPECT_EQ(transform->getPosition(), Vec2(100.0f, 200.0f));
    EXPECT_EQ(transform->getVelocity(), Vec2(5.0f, -3.0f));
    EXPECT_EQ(transform->getScale(), Vec2(2.0f, 2.0f));
    EXPECT_FLOAT_EQ(transform->getRotation(), 45.0f);
}

TEST_F(EntityTest, DeserializeNameComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cName": {
                    "name": "MyEntityName"
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto name = entity->getComponent<CName>();
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->getName(), "MyEntityName");
}

TEST_F(EntityTest, DeserializeRenderableComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cRenderable": {
                    "visualType": "Sprite",
                    "color": {"r": 255, "g": 128, "b": 64, "a": 255},
                    "zIndex": 5,
                    "visible": true
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto renderable = entity->getComponent<CRenderable>();
    ASSERT_NE(renderable, nullptr);
    EXPECT_EQ(renderable->getZIndex(), 5);
    EXPECT_TRUE(renderable->isVisible());
}

TEST_F(EntityTest, DeserializeTextureComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cTexture": {
                    "texturePath": "assets/player.png"
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto texture = entity->getComponent<CTexture>();
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->getTexturePath(), "assets/player.png");
}

TEST_F(EntityTest, DeserializeShaderComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cShader": {
                    "vertexShaderPath": "shaders/vertex.glsl",
                    "fragmentShaderPath": "shaders/fragment.glsl"
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto shader = entity->getComponent<CShader>();
    ASSERT_NE(shader, nullptr);
    EXPECT_EQ(shader->getVertexShaderPath(), "shaders/vertex.glsl");
    EXPECT_EQ(shader->getFragmentShaderPath(), "shaders/fragment.glsl");
}

TEST_F(EntityTest, DeserializeMaterialComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cMaterial": {
                    "blendMode": "Alpha",
                    "opacity": 0.8,
                    "tintColor": {"r": 255, "g": 255, "b": 255, "a": 255}
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto material = entity->getComponent<CMaterial>();
    ASSERT_NE(material, nullptr);
    EXPECT_FLOAT_EQ(material->getOpacity(), 0.8f);
}

TEST_F(EntityTest, DeserializeInputControllerComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cInputController": {
                    "actions": [
                        {
                            "action": "Jump",
                            "keys": ["57"],
                            "mouse": [],
                            "trigger": "Pressed",
                            "allowRepeat": false
                        }
                    ]
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto controller = entity->getComponent<CInputController>();
    ASSERT_NE(controller, nullptr);
}

TEST_F(EntityTest, DeserializeParticleEmitterComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cParticleEmitter": {
                    "emissionRate": 50.0,
                    "maxParticles": 200,
                    "active": true,
                    "zIndex": 10
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto emitter = entity->getComponent<CParticleEmitter>();
    ASSERT_NE(emitter, nullptr);
    EXPECT_FLOAT_EQ(emitter->getEmissionRate(), 50.0f);
    EXPECT_EQ(emitter->getMaxParticles(), 200);
    EXPECT_TRUE(emitter->isActive());
    EXPECT_EQ(emitter->getZIndex(), 10);
}

TEST_F(EntityTest, DeserializeAudioSourceComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "type": "CAudioSource",
                "data": {
                    "clip": "explosion_sfx",
                    "audioType": "sfx",
                    "volume": 0.8,
                    "pitch": 1.0,
                    "loop": false,
                    "spatial": false,
                    "playOnAwake": false,
                    "minDistance": 1.0,
                    "attenuation": 1.0
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto audioSource = entity->getComponent<CAudioSource>();
    ASSERT_NE(audioSource, nullptr);
    EXPECT_EQ(audioSource->getClipId(), "explosion_sfx");
    EXPECT_FLOAT_EQ(audioSource->getVolume(), 0.8f);
    EXPECT_FALSE(audioSource->isLooping());
}

TEST_F(EntityTest, DeserializeAudioListenerComponent)
{
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "type": "CAudioListener",
                "data": {
                    "isDefaultListener": true,
                    "audioSources": []
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    auto audioListener = entity->getComponent<CAudioListener>();
    ASSERT_NE(audioListener, nullptr);
    EXPECT_TRUE(audioListener->isDefaultListener());
}

// =============================================================================
// Entity Deserialization - Component Initialization Order Tests
// =============================================================================

TEST_F(EntityTest, DeserializePhysicsBodyAndCollider)
{
    // This test verifies that physics body is initialized before collider
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "physics_entity",
        "id": 1,
        "components": [
            {
                "cTransform": {
                    "position": {"x": 0.0, "y": 0.0},
                    "velocity": {"x": 0.0, "y": 0.0},
                    "scale": {"x": 1.0, "y": 1.0},
                    "rotation": 0.0
                }
            },
            {
                "cPhysicsBody2D": {
                    "bodyType": "Dynamic",
                    "density": 1.0,
                    "friction": 0.3,
                    "restitution": 0.1,
                    "fixedRotation": false,
                    "linearDamping": 0.0,
                    "angularDamping": 0.01,
                    "gravityScale": 1.0
                }
            },
            {
                "cCollider2D": {
                    "fixtures": [
                        {
                            "shapeType": "Circle",
                            "radius": 1.0,
                            "density": 1.0,
                            "friction": 0.3,
                            "restitution": 0.1,
                            "isSensor": false
                        }
                    ]
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("physics_entity");
    entity->deserialize(root);
    
    // Verify physics body was created
    auto physicsBody = entity->getComponent<CPhysicsBody2D>();
    ASSERT_NE(physicsBody, nullptr);
    EXPECT_EQ(physicsBody->getBodyType(), BodyType::Dynamic);
    
    // Verify collider was created and attached to the body
    auto collider = entity->getComponent<CCollider2D>();
    ASSERT_NE(collider, nullptr);
}

TEST_F(EntityTest, DeserializeColliderBeforePhysicsBody_StillWorks)
{
    // Test that even if collider appears before physics body in JSON,
    // the initialization order is correct (physics body first)
    std::string json = R"({
        "guid": "test-guid-123",
        "tag": "physics_entity",
        "id": 1,
        "components": [
            {
                "cCollider2D": {
                    "fixtures": [
                        {
                            "shapeType": "Box",
                            "size": {"x": 2.0, "y": 2.0},
                            "density": 1.0,
                            "friction": 0.5,
                            "restitution": 0.2,
                            "isSensor": false
                        }
                    ]
                }
            },
            {
                "cPhysicsBody2D": {
                    "bodyType": "Static",
                    "density": 1.0,
                    "friction": 0.5,
                    "restitution": 0.2,
                    "fixedRotation": true,
                    "linearDamping": 0.0,
                    "angularDamping": 0.0,
                    "gravityScale": 0.0
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("physics_entity");
    entity->deserialize(root);
    
    // Both components should exist
    auto physicsBody = entity->getComponent<CPhysicsBody2D>();
    auto collider = entity->getComponent<CCollider2D>();
    ASSERT_NE(physicsBody, nullptr);
    ASSERT_NE(collider, nullptr);
    EXPECT_EQ(physicsBody->getBodyType(), BodyType::Static);
}

// =============================================================================
// Entity Deserialization - Multiple Component Types Tests
// =============================================================================

TEST_F(EntityTest, DeserializeAllComponentTypes)
{
    // Test deserializing an entity with many different component types
    std::string json = R"({
        "guid": "test-guid-full",
        "tag": "full_entity",
        "id": 42,
        "components": [
            {
                "cTransform": {
                    "position": {"x": 100.0, "y": 200.0},
                    "velocity": {"x": 0.0, "y": 0.0},
                    "scale": {"x": 1.0, "y": 1.0},
                    "rotation": 0.0
                }
            },
            {
                "cName": {
                    "name": "FullTestEntity"
                }
            },
            {
                "cRenderable": {
                    "visualType": "Sprite",
                    "color": {"r": 255, "g": 255, "b": 255, "a": 255},
                    "zIndex": 0,
                    "visible": true
                }
            },
            {
                "cTexture": {
                    "texturePath": "assets/test.png"
                }
            },
            {
                "cMaterial": {
                    "blendMode": "Alpha",
                    "opacity": 1.0,
                    "tintColor": {"r": 255, "g": 255, "b": 255, "a": 255}
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("full_entity");
    entity->deserialize(root);
    
    // Verify all components were created
    EXPECT_NE(entity->getComponent<CTransform>(), nullptr);
    EXPECT_NE(entity->getComponent<CName>(), nullptr);
    EXPECT_NE(entity->getComponent<CRenderable>(), nullptr);
    EXPECT_NE(entity->getComponent<CTexture>(), nullptr);
    EXPECT_NE(entity->getComponent<CMaterial>(), nullptr);
    
    // Verify specific values
    EXPECT_EQ(entity->getComponent<CName>()->getName(), "FullTestEntity");
    EXPECT_EQ(entity->getComponent<CTransform>()->getPosition(), Vec2(100.0f, 200.0f));
    EXPECT_EQ(entity->getComponent<CTexture>()->getTexturePath(), "assets/test.png");
}

TEST_F(EntityTest, DeserializePreservesGuid)
{
    std::string json = R"({
        "guid": "preserved-guid-12345",
        "tag": "test",
        "id": 1,
        "components": []
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    EXPECT_EQ(entity->getGuid(), "preserved-guid-12345");
}

TEST_F(EntityTest, DeserializePreservesId)
{
    std::string json = R"({
        "guid": "test-guid",
        "tag": "test",
        "id": 999,
        "components": []
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    EXPECT_EQ(entity->getId(), 999);
}

// =============================================================================
// Entity Serialization/Deserialization Round Trip Tests
// =============================================================================

TEST_F(EntityTest, SerializeDeserializeRoundTrip)
{
    // Create entity with components
    auto entity1 = SEntity::instance().addEntity("round_trip");
    entity1->addComponent<CTransform>()->setPosition(Vec2(50.0f, 100.0f));
    entity1->addComponent<CName>()->setName("RoundTripTest");
    entity1->addComponent<CRenderable>()->setZIndex(5);
    
    // Serialize
    Serialization::JsonBuilder builder;
    entity1->serialize(builder);
    std::string json = builder.toString();
    
    // Deserialize into a new entity
   Serialization::SSerialization::JsonValue root(json);
    auto entity2 = SEntity::instance().addEntity("round_trip_copy");
    entity2->deserialize(root);
    
    // Verify round trip preserves values
    auto transform1 = entity1->getComponent<CTransform>();
    auto transform2 = entity2->getComponent<CTransform>();
    ASSERT_NE(transform2, nullptr);
    EXPECT_EQ(transform1->getPosition(), transform2->getPosition());
    
    auto name1 = entity1->getComponent<CName>();
    auto name2 = entity2->getComponent<CName>();
    ASSERT_NE(name2, nullptr);
    EXPECT_EQ(name1->getName(), name2->getName());
    
    auto renderable1 = entity1->getComponent<CRenderable>();
    auto renderable2 = entity2->getComponent<CRenderable>();
    ASSERT_NE(renderable2, nullptr);
    EXPECT_EQ(renderable1->getZIndex(), renderable2->getZIndex());
}

TEST_F(EntityTest, SerializeDeserializePhysicsRoundTrip)
{
    // Create entity with physics components
    auto entity1 = SEntity::instance().addEntity("physics_round_trip");
    auto transform1 = entity1->addComponent<CTransform>();
    transform1->setPosition(Vec2(0.0f, 0.0f));
    
    auto physics1 = entity1->addComponent<CPhysicsBody2D>();
    physics1->initialize({0.0f, 0.0f});
    physics1->setBodyType(BodyType::Dynamic);
    physics1->setDensity(2.0f);
    physics1->setFriction(0.7f);
    physics1->setRestitution(0.3f);
    physics1->setFixedRotation(true);
    
    auto collider1 = entity1->addComponent<CCollider2D>();
    collider1->createBox(1.0f, 1.0f);
    
    // Serialize
    Serialization::JsonBuilder builder;
    entity1->serialize(builder);
    std::string json = builder.toString();
    
    // Deserialize into a new entity
   Serialization::SSerialization::JsonValue root(json);
    auto entity2 = SEntity::instance().addEntity("physics_round_trip_copy");
    entity2->deserialize(root);
    
    // Verify physics body
    auto physics2 = entity2->getComponent<CPhysicsBody2D>();
    ASSERT_NE(physics2, nullptr);
    EXPECT_EQ(physics2->getBodyType(), BodyType::Dynamic);
    EXPECT_FLOAT_EQ(physics2->getDensity(), 2.0f);
    EXPECT_FLOAT_EQ(physics2->getFriction(), 0.7f);
    EXPECT_FLOAT_EQ(physics2->getRestitution(), 0.3f);
    EXPECT_TRUE(physics2->isFixedRotation());
    
    // Verify collider exists
    auto collider2 = entity2->getComponent<CCollider2D>();
    ASSERT_NE(collider2, nullptr);
}

TEST_F(EntityTest, SerializeDeserializePreservesComponentGuids)
{
    // Create entity with components that have GUIDs referenced by other components
    auto entity1 = SEntity::instance().addEntity("guid_test");
    auto texture1 = entity1->addComponent<CTexture>();
    texture1->setTexturePath("assets/test.png");
    auto shader1 = entity1->addComponent<CShader>();
    shader1->setVertexShaderPath("shaders/vertex.glsl");
    shader1->setFragmentShaderPath("shaders/fragment.glsl");
    auto material1 = entity1->addComponent<CMaterial>();
    material1->setTextureGuid(texture1->getGuid());
    material1->setShaderGuid(shader1->getGuid());
    
    // Store original GUIDs
    std::string originalTextureGuid = texture1->getGuid();
    std::string originalShaderGuid = shader1->getGuid();
    
    // Serialize
    Serialization::JsonBuilder builder;
    entity1->serialize(builder);
    std::string json = builder.toString();
    
    // Deserialize into a new entity
   Serialization::SSerialization::JsonValue root(json);
    auto entity2 = SEntity::instance().addEntity("guid_test_copy");
    entity2->deserialize(root);
    
    // Verify component GUIDs are preserved
    auto texture2 = entity2->getComponent<CTexture>();
    auto shader2 = entity2->getComponent<CShader>();
    auto material2 = entity2->getComponent<CMaterial>();
    
    ASSERT_NE(texture2, nullptr);
    ASSERT_NE(shader2, nullptr);
    ASSERT_NE(material2, nullptr);
    
    // GUIDs should be preserved through serialization
    EXPECT_EQ(texture2->getGuid(), originalTextureGuid);
    EXPECT_EQ(shader2->getGuid(), originalShaderGuid);
    
    // Material references should still be valid
    EXPECT_EQ(material2->getTextureGuid(), originalTextureGuid);
    EXPECT_EQ(material2->getShaderGuid(), originalShaderGuid);
}

// =============================================================================
// Edge Cases and Error Handling
// =============================================================================

TEST_F(EntityTest, DeserializeEmptyComponents)
{
    std::string json = R"({
        "guid": "test-guid",
        "tag": "empty",
        "id": 1,
        "components": []
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("empty");
    entity->deserialize(root);
    
    // Entity should have no components
    EXPECT_FALSE(entity->hasComponent<CTransform>());
    EXPECT_FALSE(entity->hasComponent<CName>());
}

TEST_F(EntityTest, DeserializeUnknownComponentType)
{
    // Unknown component types should be skipped gracefully
    std::string json = R"({
        "guid": "test-guid",
        "tag": "test",
        "id": 1,
        "components": [
            {
                "cUnknownComponent": {
                    "someField": "someValue"
                }
            },
            {
                "cName": {
                    "name": "ValidComponent"
                }
            }
        ]
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    entity->deserialize(root);
    
    // Unknown component should be skipped, valid one should work
    auto name = entity->getComponent<CName>();
    ASSERT_NE(name, nullptr);
    EXPECT_EQ(name->getName(), "ValidComponent");
}

TEST_F(EntityTest, DeserializeWithoutGuid)
{
    // Test that deserialization works even if guid is missing (uses existing)
    std::string json = R"({
        "tag": "test",
        "id": 1,
        "components": []
    })";
    
   Serialization::SSerialization::JsonValue root(json);
    auto entity = SEntity::instance().addEntity("test");
    std::string originalGuid = entity->getGuid();
    entity->deserialize(root);
    
    // Should keep the original GUID
    EXPECT_EQ(entity->getGuid(), originalGuid);
}
