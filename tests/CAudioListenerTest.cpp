#include <gtest/gtest.h>
#include "CAudioListener.h"
#include "CTransform.h"
#include "Entity.h"
#include "SEntity.h"
#include "SAudio.h"
#include "SSerialization.h"

class CAudioListenerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SAudio::instance().initialize();
        entity = SEntity::instance().addEntity("listener");
    }

    void TearDown() override
    {
        SEntity::instance().clear();
        SAudio::instance().shutdown();
    }

    std::shared_ptr<Entity> entity;
};

TEST_F(CAudioListenerTest, ComponentCreationAndDefaults)
{
    auto* listener = entity->addComponent<CAudioListener>();

    EXPECT_NE(listener, nullptr);
    EXPECT_EQ(listener->getType(), "AudioListener");
    EXPECT_TRUE(listener->isDefaultListener());
    EXPECT_TRUE(listener->getAudioSources().empty());
}

TEST_F(CAudioListenerTest, DefaultListenerFlag)
{
    auto* listener = entity->addComponent<CAudioListener>();

    listener->setDefaultListener(false);
    EXPECT_FALSE(listener->isDefaultListener());

    listener->setDefaultListener(true);
    EXPECT_TRUE(listener->isDefaultListener());
}

TEST_F(CAudioListenerTest, AddAndRemoveAudioSource)
{
    auto* listener = entity->addComponent<CAudioListener>();

    CAudioListener::AudioSourceConfig config;
    config.clipId = "footstep";
    config.type   = AudioType::SFX;
    config.volume = 0.7f;

    listener->addAudioSource("walk", config);

    const auto& sources = listener->getAudioSources();
    EXPECT_EQ(sources.size(), 1);
    EXPECT_TRUE(sources.find("walk") != sources.end());
    EXPECT_EQ(sources.at("walk").clipId, "footstep");

    listener->removeAudioSource("walk");
    EXPECT_TRUE(listener->getAudioSources().empty());
}

TEST_F(CAudioListenerTest, PlayNonExistentSource)
{
    auto* listener = entity->addComponent<CAudioListener>();

    // Should fail gracefully
    EXPECT_FALSE(listener->play("non_existent"));
}

TEST_F(CAudioListenerTest, IsPlayingReturnsFalseForNonExistent)
{
    auto* listener = entity->addComponent<CAudioListener>();

    EXPECT_FALSE(listener->isPlaying("non_existent"));
}

TEST_F(CAudioListenerTest, PauseStopNonExistentSource)
{
    auto* listener = entity->addComponent<CAudioListener>();

    // Should not crash
    listener->pause("non_existent");
    listener->stop("non_existent");
}

TEST_F(CAudioListenerTest, Serialization)
{
    auto* listener = entity->addComponent<CAudioListener>();
    listener->setDefaultListener(true);

    CAudioListener::AudioSourceConfig config1;
    config1.clipId      = "jump";
    config1.type        = AudioType::SFX;
    config1.volume      = 0.8f;
    config1.pitch       = 1.2f;
    config1.loop        = false;
    config1.spatial     = true;
    config1.minDistance = 5.0f;
    config1.attenuation = 2.0f;

    CAudioListener::AudioSourceConfig config2;
    config2.clipId = "ambient";
    config2.type   = AudioType::Music;
    config2.volume = 0.5f;
    config2.loop   = true;

    listener->addAudioSource("jump", config1);
    listener->addAudioSource("ambient", config2);

    Serialization::JsonBuilder builder;
    listener->serialize(builder);
    std::string json = builder.toString();

    EXPECT_TRUE(json.find("\"isDefaultListener\":true") != std::string::npos);
    EXPECT_TRUE(json.find("\"audioSources\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"jump\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"ambient\"") != std::string::npos);
}

TEST_F(CAudioListenerTest, Deserialization)
{
    std::string json = R"({
        "type": "AudioListener",
        "data": {
            "isDefaultListener": false,
            "audioSources": [
                {
                    "name": "shoot",
                    "clip": "laser",
                    "audioType": "sfx",
                    "volume": 0.9,
                    "pitch": 1.5,
                    "loop": false,
                    "spatial": true,
                    "minDistance": 3.0,
                    "attenuation": 1.5
                },
                {
                    "name": "bgm",
                    "clip": "theme",
                    "audioType": "music",
                    "volume": 0.6,
                    "pitch": 1.0,
                    "loop": true,
                    "spatial": false,
                    "minDistance": 1.0,
                    "attenuation": 1.0
                }
            ]
        }
    })";    Serialization::SSerialization::JsonValue value(json);

    auto* listener = entity->addComponent<CAudioListener>();
    listener->deserialize(value);

    EXPECT_FALSE(listener->isDefaultListener());

    const auto& sources = listener->getAudioSources();
    EXPECT_EQ(sources.size(), 2);

    EXPECT_TRUE(sources.find("shoot") != sources.end());
    const auto& shootConfig = sources.at("shoot");
    EXPECT_EQ(shootConfig.clipId, "laser");
    EXPECT_EQ(shootConfig.type, AudioType::SFX);
    EXPECT_FLOAT_EQ(shootConfig.volume, 0.9f);
    EXPECT_FLOAT_EQ(shootConfig.pitch, 1.5f);
    EXPECT_FALSE(shootConfig.loop);
    EXPECT_TRUE(shootConfig.spatial);
    EXPECT_FLOAT_EQ(shootConfig.minDistance, 3.0f);
    EXPECT_FLOAT_EQ(shootConfig.attenuation, 1.5f);

    EXPECT_TRUE(sources.find("bgm") != sources.end());
    const auto& bgmConfig = sources.at("bgm");
    EXPECT_EQ(bgmConfig.clipId, "theme");
    EXPECT_EQ(bgmConfig.type, AudioType::Music);
    EXPECT_FLOAT_EQ(bgmConfig.volume, 0.6f);
    EXPECT_TRUE(bgmConfig.loop);
    EXPECT_FALSE(bgmConfig.spatial);
}

TEST_F(CAudioListenerTest, UpdateWithTransform)
{
    auto* transform = entity->addComponent<CTransform>();
    transform->setPosition(Vec2(100.0f, 200.0f));

    auto* listener = entity->addComponent<CAudioListener>();
    listener->init();

    // Update should sync listener position (should not crash)
    listener->update(0.016f);
}

TEST_F(CAudioListenerTest, UpdateWithoutTransform)
{
    auto* listener = entity->addComponent<CAudioListener>();
    listener->init();

    // Should handle missing transform gracefully
    listener->update(0.016f);
}

TEST_F(CAudioListenerTest, MultipleAudioSources)
{
    auto* listener = entity->addComponent<CAudioListener>();

    // Add multiple sources
    for (int i = 0; i < 10; ++i)
    {
        CAudioListener::AudioSourceConfig config;
        config.clipId = "sound" + std::to_string(i);
        config.type   = AudioType::SFX;
        listener->addAudioSource("source" + std::to_string(i), config);
    }

    EXPECT_EQ(listener->getAudioSources().size(), 10);

    // Remove some sources
    listener->removeAudioSource("source3");
    listener->removeAudioSource("source7");

    EXPECT_EQ(listener->getAudioSources().size(), 8);
}

TEST_F(CAudioListenerTest, PlaySourceWithoutClipId)
{
    auto* listener = entity->addComponent<CAudioListener>();

    CAudioListener::AudioSourceConfig config;
    config.clipId = "";  // Empty clip ID
    config.type   = AudioType::SFX;

    listener->addAudioSource("empty", config);

    // Should fail gracefully
    EXPECT_FALSE(listener->play("empty"));
}
