#include <gtest/gtest.h>
#include "CAudioSource.h"
#include "CTransform.h"
#include "Entity.h"
#include "EntityManager.h"
#include "SAudioSystem.h"
#include "systems/SSerialization.h"

class CAudioSourceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SAudioSystem::instance().initialize();
        entity = EntityManager::instance().addEntity("test");
    }

    void TearDown() override
    {
        EntityManager::instance().clear();
        SAudioSystem::instance().shutdown();
    }

    std::shared_ptr<Entity> entity;
};

TEST_F(CAudioSourceTest, ComponentCreationAndDefaults)
{
    auto* audioSource = entity->addComponent<CAudioSource>();

    EXPECT_NE(audioSource, nullptr);
    EXPECT_EQ(audioSource->getType(), "AudioSource");
    EXPECT_TRUE(audioSource->getClipId().empty());
    EXPECT_EQ(audioSource->getAudioType(), AudioType::SFX);
    EXPECT_FLOAT_EQ(audioSource->getVolume(), AudioConstants::DEFAULT_SFX_VOLUME);
    EXPECT_FLOAT_EQ(audioSource->getPitch(), AudioConstants::DEFAULT_AUDIO_PITCH);
    EXPECT_FALSE(audioSource->isLooping());
    EXPECT_FALSE(audioSource->isSpatial());
}

TEST_F(CAudioSourceTest, SettersAndGetters)
{
    auto* audioSource = entity->addComponent<CAudioSource>();

    audioSource->setClipId("test_clip");
    EXPECT_EQ(audioSource->getClipId(), "test_clip");

    audioSource->setAudioType(AudioType::Music);
    EXPECT_EQ(audioSource->getAudioType(), AudioType::Music);

    audioSource->setVolume(0.5f);
    EXPECT_FLOAT_EQ(audioSource->getVolume(), 0.5f);

    audioSource->setPitch(1.5f);
    EXPECT_FLOAT_EQ(audioSource->getPitch(), 1.5f);

    audioSource->setLoop(true);
    EXPECT_TRUE(audioSource->isLooping());

    audioSource->setSpatial(true);
    EXPECT_TRUE(audioSource->isSpatial());

    audioSource->setPlayOnAwake(true);
    // No getter for playOnAwake, but we can test it through serialization
}

TEST_F(CAudioSourceTest, Serialization)
{
    auto* audioSource = entity->addComponent<CAudioSource>();
    audioSource->setClipId("jump_sound");
    audioSource->setAudioType(AudioType::SFX);
    audioSource->setVolume(0.8f);
    audioSource->setPitch(1.2f);
    audioSource->setLoop(true);
    audioSource->setSpatial(true);
    audioSource->setPlayOnAwake(false);

    JsonBuilder builder;
    audioSource->serialize(builder);
    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"clip\":\"jump_sound\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"audioType\":\"sfx\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"volume\":0.8") != std::string::npos);
    EXPECT_TRUE(json.find("\"loop\":true") != std::string::npos);
    EXPECT_TRUE(json.find("\"spatial\":true") != std::string::npos);
}

TEST_F(CAudioSourceTest, Deserialization)
{
    std::string json = R"({
        "type": "AudioSource",
        "data": {
            "clip": "explosion",
            "audioType": "sfx",
            "volume": 0.9,
            "pitch": 0.8,
            "loop": false,
            "spatial": true,
            "playOnAwake": true,
            "minDistance": 5.0,
            "attenuation": 2.0
        }
    })";

    JsonParser parser(json);
    JsonValue  value = JsonValue::parse(parser);

    auto* audioSource = entity->addComponent<CAudioSource>();
    audioSource->deserialize(value);

    EXPECT_EQ(audioSource->getClipId(), "explosion");
    EXPECT_EQ(audioSource->getAudioType(), AudioType::SFX);
    EXPECT_FLOAT_EQ(audioSource->getVolume(), 0.9f);
    EXPECT_FLOAT_EQ(audioSource->getPitch(), 0.8f);
    EXPECT_FALSE(audioSource->isLooping());
    EXPECT_TRUE(audioSource->isSpatial());
}

TEST_F(CAudioSourceTest, DeserializationMusicType)
{
    std::string json = R"({
        "type": "AudioSource",
        "data": {
            "clip": "background_music",
            "audioType": "music",
            "volume": 0.6,
            "loop": true
        }
    })";

    JsonParser parser(json);
    JsonValue  value = JsonValue::parse(parser);

    auto* audioSource = entity->addComponent<CAudioSource>();
    audioSource->deserialize(value);

    EXPECT_EQ(audioSource->getClipId(), "background_music");
    EXPECT_EQ(audioSource->getAudioType(), AudioType::Music);
    EXPECT_FLOAT_EQ(audioSource->getVolume(), 0.6f);
    EXPECT_TRUE(audioSource->isLooping());
}

TEST_F(CAudioSourceTest, PlayWithoutClipId)
{
    auto* audioSource = entity->addComponent<CAudioSource>();

    // Should fail gracefully when no clip ID is set
    EXPECT_FALSE(audioSource->play());
}

TEST_F(CAudioSourceTest, IsPlayingReturnsFalseInitially)
{
    auto* audioSource = entity->addComponent<CAudioSource>();
    audioSource->setClipId("test");

    EXPECT_FALSE(audioSource->isPlaying());
}

TEST_F(CAudioSourceTest, UpdateWithTransform)
{
    // Add transform component
    auto* transform = entity->addComponent<CTransform>();
    transform->setPosition(Vec2(10.0f, 20.0f));

    auto* audioSource = entity->addComponent<CAudioSource>();
    audioSource->setClipId("test");
    audioSource->setSpatial(true);

    // Update should not crash even without playing sound
    audioSource->update(0.016f);
}

TEST_F(CAudioSourceTest, VolumeControlClamping)
{
    auto* audioSource = entity->addComponent<CAudioSource>();

    // Test upper bound
    audioSource->setVolume(2.0f);
    EXPECT_FLOAT_EQ(audioSource->getVolume(), 1.0f);

    // Test lower bound
    audioSource->setVolume(-0.5f);
    EXPECT_FLOAT_EQ(audioSource->getVolume(), 0.0f);

    // Test normal value
    audioSource->setVolume(0.5f);
    EXPECT_FLOAT_EQ(audioSource->getVolume(), 0.5f);
}
