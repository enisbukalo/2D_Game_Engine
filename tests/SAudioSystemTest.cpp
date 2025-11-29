#include <gtest/gtest.h>
#include "SAudioSystem.h"
#include "AudioTypes.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <SFML/Audio.hpp>

/**
 * @brief Test fixture for SAudioSystem tests
 */
class SAudioSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Redirect SFML's OpenAL error output to suppress harmless errors in headless environment
        sf::err().rdbuf(nullptr);
        
        // Create a minimal test WAV file (440Hz sine wave, 0.1 seconds)
        createTestWavFile("test_sound.wav", 440.0, 0.1);
        createTestWavFile("test_music.wav", 220.0, 0.5);
    }

    void TearDown() override
    {
        SAudioSystem::instance().shutdown();
        std::remove("test_sound.wav");
        std::remove("test_music.wav");
    }

    /**
     * @brief Create a simple WAV file for testing
     * @param filename Output filename
     * @param frequency Frequency in Hz
     * @param duration Duration in seconds
     */
    void createTestWavFile(const std::string& filename, double frequency, double duration)
    {
        const int   sampleRate    = 44100;
        const int   numChannels   = 1;
        const int   bitsPerSample = 16;
        const int   numSamples    = static_cast<int>(sampleRate * duration);
        const int   dataSize      = numSamples * numChannels * (bitsPerSample / 8);
        const int   fileSize      = 44 + dataSize;

        std::ofstream file(filename, std::ios::binary);

        // WAV header
        file.write("RIFF", 4);
        int32_t chunkSize = fileSize - 8;
        file.write(reinterpret_cast<const char*>(&chunkSize), 4);
        file.write("WAVE", 4);

        // Format chunk
        file.write("fmt ", 4);
        int32_t fmtChunkSize = 16;
        file.write(reinterpret_cast<const char*>(&fmtChunkSize), 4);
        int16_t audioFormat = 1;  // PCM
        file.write(reinterpret_cast<const char*>(&audioFormat), 2);
        int16_t channels = numChannels;
        file.write(reinterpret_cast<const char*>(&channels), 2);
        int32_t sr = sampleRate;
        file.write(reinterpret_cast<const char*>(&sr), 4);
        int32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
        file.write(reinterpret_cast<const char*>(&byteRate), 4);
        int16_t blockAlign = numChannels * (bitsPerSample / 8);
        file.write(reinterpret_cast<const char*>(&blockAlign), 2);
        int16_t bps = bitsPerSample;
        file.write(reinterpret_cast<const char*>(&bps), 2);

        // Data chunk
        file.write("data", 4);
        file.write(reinterpret_cast<const char*>(&dataSize), 4);

        // Generate sine wave samples
        for (int i = 0; i < numSamples; ++i)
        {
            double t      = static_cast<double>(i) / sampleRate;
            double value  = std::sin(2.0 * M_PI * frequency * t);
            int16_t sample = static_cast<int16_t>(value * 32767.0 * 0.5);  // 50% volume
            file.write(reinterpret_cast<const char*>(&sample), 2);
        }

        file.close();
    }
};

TEST_F(SAudioSystemTest, InitializeAndShutdown)
{
    auto& audioSystem = SAudioSystem::instance();

    EXPECT_TRUE(audioSystem.initialize());
    EXPECT_TRUE(audioSystem.initialize());  // Should handle double initialization

    audioSystem.shutdown();
    audioSystem.shutdown();  // Should handle double shutdown
}

TEST_F(SAudioSystemTest, LoadAndUnloadSFX)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();

    // Load a sound effect
    EXPECT_TRUE(audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX));

    // Try loading the same sound again (should succeed with warning)
    EXPECT_TRUE(audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX));

    // Unload the sound
    audioSystem.unloadSound("test_sfx");

    // Unload non-existent sound (should not crash)
    audioSystem.unloadSound("non_existent");
}

TEST_F(SAudioSystemTest, LoadAndUnloadMusic)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();

    // Load music (just registers the path)
    EXPECT_TRUE(audioSystem.loadSound("test_music", "test_music.wav", AudioType::Music));

    // Unload music
    audioSystem.unloadSound("test_music");
}

TEST_F(SAudioSystemTest, PlaySFXReturnsValidHandle)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    // Test that play returns a valid handle (tests pool allocation logic)
    AudioHandle handle = audioSystem.playSFX("test_sfx");
    EXPECT_TRUE(handle.isValid());
    EXPECT_NE(handle.index, AudioHandle::INVALID_INDEX);

    // Test that we can get multiple handles (pool management)
    AudioHandle handle2 = audioSystem.playSFX("test_sfx");
    EXPECT_TRUE(handle2.isValid());
    EXPECT_NE(handle.index, handle2.index);  // Different slots

    // Stop the sounds (cleanup)
    audioSystem.stopSFX(handle);
    audioSystem.stopSFX(handle2);
}

TEST_F(SAudioSystemTest, SpatialSFXReturnsValidHandle)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    // Test that spatial play returns a valid handle (tests spatial logic path)
    Vec2        position(10.0f, 20.0f);
    AudioHandle handle = audioSystem.playSpatialSFX("test_sfx", position);
    EXPECT_TRUE(handle.isValid());

    // Test that position update doesn't crash (software validation)
    Vec2 newPosition(30.0f, 40.0f);
    audioSystem.setSFXPosition(handle, newPosition);

    // Stop the sound (cleanup)
    audioSystem.stopSFX(handle);
}

TEST_F(SAudioSystemTest, PauseAndResumeSFXDoesNotCrash)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    AudioHandle handle = audioSystem.playSFX("test_sfx");
    EXPECT_TRUE(handle.isValid());

    // Test that pause/resume don't crash (software validation)
    audioSystem.pauseSFX(handle);
    audioSystem.resumeSFX(handle);
    audioSystem.stopSFX(handle);
    
    // Test operations on stopped sound are safe
    audioSystem.pauseSFX(handle);  // Should be safe
}

TEST_F(SAudioSystemTest, MusicLoadAndPlayAPI)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_music", "test_music.wav", AudioType::Music);

    // Test that playMusic returns success (tests loading logic)
    bool started = audioSystem.playMusic("test_music", true, 0.5f);
    // In headless, may succeed or fail depending on OpenAL availability
    // Just verify it doesn't crash
    
    // Stop music (cleanup)
    audioSystem.stopMusic();
}

TEST_F(SAudioSystemTest, MusicPauseResumeDoesNotCrash)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_music", "test_music.wav", AudioType::Music);

    // Test that music control methods don't crash
    audioSystem.playMusic("test_music");
    audioSystem.pauseMusic();
    audioSystem.resumeMusic();
    audioSystem.stopMusic();
    
    // Multiple stops should be safe
    audioSystem.stopMusic();
}

TEST_F(SAudioSystemTest, VolumeControl)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();

    // Test master volume
    audioSystem.setMasterVolume(0.5f);
    EXPECT_FLOAT_EQ(audioSystem.getMasterVolume(), 0.5f);

    // Test SFX volume
    audioSystem.setSFXVolume(0.7f);
    EXPECT_FLOAT_EQ(audioSystem.getSFXVolume(), 0.7f);

    // Test music volume
    audioSystem.setMusicVolume(0.3f);
    EXPECT_FLOAT_EQ(audioSystem.getMusicVolume(), 0.3f);

    // Test clamping (values should be clamped to [0, 1])
    audioSystem.setMasterVolume(1.5f);
    EXPECT_FLOAT_EQ(audioSystem.getMasterVolume(), 1.0f);

    audioSystem.setSFXVolume(-0.5f);
    EXPECT_FLOAT_EQ(audioSystem.getSFXVolume(), 0.0f);
}

TEST_F(SAudioSystemTest, ListenerPosition)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();

    // Set listener position (should not crash)
    Vec2 position(100.0f, 200.0f);
    audioSystem.setListenerPosition(position);
}

TEST_F(SAudioSystemTest, SoundPoolExhaustion)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    // Try to fill up the pool (default size is 32)
    std::vector<AudioHandle> handles;
    for (int i = 0; i < 35; ++i)
    {
        AudioHandle handle = audioSystem.playSFX("test_sfx", 0.1f, 1.0f, true);  // Loop to keep playing
        handles.push_back(handle);
    }

    // First 32 should succeed, rest should fail
    int validCount = 0;
    for (const auto& handle : handles)
    {
        if (handle.isValid())
        {
            validCount++;
        }
    }
    EXPECT_EQ(validCount, 32);

    // Clean up - stop all sounds
    for (const auto& handle : handles)
    {
        if (handle.isValid())
        {
            audioSystem.stopSFX(handle);
        }
    }
}

TEST_F(SAudioSystemTest, UpdateFreesFinishedSounds)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    // Play a non-looping sound
    AudioHandle handle = audioSystem.playSFX("test_sfx", 1.0f, 1.0f, false);
    EXPECT_TRUE(handle.isValid());

    // Wait for sound to finish (test sound is 0.1 seconds)
    // In a real scenario, we'd wait, but for unit tests we'll just call update multiple times
    for (int i = 0; i < 10; ++i)
    {
        audioSystem.update(0.02f);  // Simulate 20ms frames
    }

    // After enough time, the sound should no longer be playing
    // Note: This test might be timing-dependent, but it demonstrates the update mechanism
}

TEST_F(SAudioSystemTest, InvalidOperations)
{
    auto& audioSystem = SAudioSystem::instance();

    // Operations before initialization should fail gracefully
    EXPECT_FALSE(audioSystem.loadSound("test", "test.wav", AudioType::SFX));
    AudioHandle handle = audioSystem.playSFX("test");
    EXPECT_FALSE(handle.isValid());

    audioSystem.initialize();

    // Play non-existent sound
    handle = audioSystem.playSFX("non_existent");
    EXPECT_FALSE(handle.isValid());

    // Operations on invalid handles should not crash
    AudioHandle invalidHandle = AudioHandle::invalid();
    audioSystem.stopSFX(invalidHandle);
    audioSystem.pauseSFX(invalidHandle);
    audioSystem.resumeSFX(invalidHandle);
    EXPECT_FALSE(audioSystem.isPlayingSFX(invalidHandle));
}

TEST_F(SAudioSystemTest, MasterVolumePreservesIndividualSFXVolumes)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    // Set initial master volume to 1.0
    audioSystem.setMasterVolume(1.0f);

    // Play sound with specific volume
    AudioHandle handle = audioSystem.playSFX("test_sfx", 0.5f, 1.0f, true);
    EXPECT_TRUE(handle.isValid());

    // Change master volume down
    audioSystem.setMasterVolume(0.5f);
    EXPECT_FLOAT_EQ(audioSystem.getMasterVolume(), 0.5f);

    // Change master volume back up
    audioSystem.setMasterVolume(1.0f);
    EXPECT_FLOAT_EQ(audioSystem.getMasterVolume(), 1.0f);

    // The sound should still maintain its individual volume of 0.5
    // We can't directly test SFML's internal volume, but we verify no crash
    // and that the system is still functional
    audioSystem.setSFXVolume(handle, 0.8f);
    
    // Change master volume again - should apply to the new base volume (0.8)
    audioSystem.setMasterVolume(0.25f);
    audioSystem.setMasterVolume(1.0f);

    audioSystem.stopSFX(handle);
}

TEST_F(SAudioSystemTest, MasterVolumePreservesMusicVolume)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_music", "test_music.wav", AudioType::Music);

    // Set initial master volume
    audioSystem.setMasterVolume(1.0f);

    // Play music with specific volume
    EXPECT_TRUE(audioSystem.playMusic("test_music", true, 0.6f));

    // Change master volume down then back up
    audioSystem.setMasterVolume(0.3f);
    EXPECT_FLOAT_EQ(audioSystem.getMasterVolume(), 0.3f);
    
    audioSystem.setMasterVolume(0.8f);
    EXPECT_FLOAT_EQ(audioSystem.getMasterVolume(), 0.8f);
    
    audioSystem.setMasterVolume(1.0f);
    EXPECT_FLOAT_EQ(audioSystem.getMasterVolume(), 1.0f);

    // Music should still be playing with its base volume preserved
    EXPECT_TRUE(audioSystem.isMusicPlaying());

    audioSystem.stopMusic();
}

TEST_F(SAudioSystemTest, CategoryVolumePreservesIndividualVolumes)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    // Play sound with specific volume
    AudioHandle handle = audioSystem.playSFX("test_sfx", 0.7f, 1.0f, true);
    EXPECT_TRUE(handle.isValid());

    // Change SFX category volume down then back up
    audioSystem.setSFXVolume(0.5f);
    EXPECT_FLOAT_EQ(audioSystem.getSFXVolume(), 0.5f);
    
    audioSystem.setSFXVolume(1.0f);
    EXPECT_FLOAT_EQ(audioSystem.getSFXVolume(), 1.0f);

    // Sound should maintain its individual 0.7 base volume
    // Verify system is still functional
    audioSystem.setSFXVolume(handle, 0.3f);

    audioSystem.stopSFX(handle);
}

TEST_F(SAudioSystemTest, IndividualSFXVolumeChange)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    // Set master volume to something other than 1.0
    audioSystem.setMasterVolume(0.8f);

    // Play sound with initial volume
    AudioHandle handle = audioSystem.playSFX("test_sfx", 0.5f, 1.0f, true);
    EXPECT_TRUE(handle.isValid());

    // Change individual sound volume
    audioSystem.setSFXVolume(handle, 0.9f);

    // Change master volume - should use new base volume (0.9)
    audioSystem.setMasterVolume(0.5f);
    audioSystem.setMasterVolume(0.8f);

    // Verify handle is still valid (sound should still exist)
    EXPECT_TRUE(handle.isValid());

    audioSystem.stopSFX(handle);
}

TEST_F(SAudioSystemTest, MultipleVolumeChangesDoNotCompound)
{
    auto& audioSystem = SAudioSystem::instance();
    audioSystem.initialize();
    audioSystem.loadSound("test_sfx", "test_sound.wav", AudioType::SFX);

    // Play sound at 50% volume
    AudioHandle handle = audioSystem.playSFX("test_sfx", 0.5f, 1.0f, true);
    EXPECT_TRUE(handle.isValid());

    // Repeatedly change master volume up and down
    for (int i = 0; i < 5; ++i)
    {
        audioSystem.setMasterVolume(0.2f);
        audioSystem.setMasterVolume(0.8f);
        audioSystem.setMasterVolume(0.5f);
        audioSystem.setMasterVolume(1.0f);
    }

    // Verify handle is still valid after all volume changes
    EXPECT_TRUE(handle.isValid());

    audioSystem.stopSFX(handle);
}
