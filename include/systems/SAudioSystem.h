#ifndef SAUDIOSYSTEM_H
#define SAUDIOSYSTEM_H

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "IAudioSystem.h"

/**
 * @brief SFML-based implementation of the audio system
 *
 * @description
 * SAudioSystem implements audio playback using SFML's audio module.
 * It manages a fixed-size pool of sf::Sound objects for SFX playback
 * and a single sf::Music object for streamed music playback.
 *
 * Features:
 * - SFX pooling: Reuses sf::Sound objects for efficient playback
 * - Music streaming: Single active music track with streaming
 * - Spatial audio: 3D positioning and attenuation for sound effects
 * - Volume control: Separate master, SFX, and music volume levels
 *
 * Thread Safety:
 * All methods should be called from the main thread. SFML audio operations
 * are not guaranteed to be thread-safe.
 */
class SAudioSystem : public IAudioSystem
{
public:
    /**
     * @brief Construct audio system with specified pool size
     * @param poolSize Number of simultaneous sound effects (default 32)
     */
    explicit SAudioSystem(size_t poolSize = AudioConstants::DEFAULT_SFX_POOL_SIZE);

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~SAudioSystem() override;

    // IAudioSystem interface implementation
    bool initialize() override;
    void shutdown() override;

    bool loadSound(const std::string& id, const std::string& filepath, AudioType type) override;
    void unloadSound(const std::string& id) override;

    AudioHandle playSFX(const std::string& id, float volume = 1.0f, float pitch = 1.0f, bool loop = false) override;
    AudioHandle playSpatialSFX(const std::string& id,
                               const Vec2&        position,
                               float              volume       = 1.0f,
                               float              pitch        = 1.0f,
                               bool               loop         = false,
                               float              minDistance  = AudioConstants::DEFAULT_MIN_DISTANCE,
                               float              attenuation  = AudioConstants::DEFAULT_ATTENUATION) override;

    void stopSFX(AudioHandle handle) override;
    void pauseSFX(AudioHandle handle) override;
    void resumeSFX(AudioHandle handle) override;
    bool isPlayingSFX(AudioHandle handle) const override;
    void setSFXPosition(AudioHandle handle, const Vec2& position) override;
    void setSFXVolume(AudioHandle handle, float volume) override;

    bool playMusic(const std::string& id, bool loop = true, float volume = 1.0f) override;
    void stopMusic() override;
    void pauseMusic() override;
    void resumeMusic() override;
    bool isMusicPlaying() const override;

    void setMasterVolume(float volume) override;
    void setSFXVolume(float volume) override;
    void setMusicVolume(float volume) override;

    float getMasterVolume() const override;
    float getSFXVolume() const override;
    float getMusicVolume() const override;

    void setListenerPosition(const Vec2& position) override;
    void update(float deltaTime) override;

    /**
     * @brief Get singleton instance
     * @return Reference to the audio system instance
     */
    static SAudioSystem& instance();

private:
    /**
     * @brief Sound pool slot with generation tracking
     */
    struct SoundSlot
    {
        sf::Sound sound;
        uint32_t  generation = 0;
        bool      inUse      = false;
    };

    /**
     * @brief Find an available slot in the sound pool
     * @return Index of available slot, or -1 if pool is full
     */
    int findAvailableSlot();

    /**
     * @brief Validate that a handle points to an active sound
     * @param handle Handle to validate
     * @return true if handle is valid and sound is in use
     */
    bool isHandleValid(AudioHandle handle) const;

    /**
     * @brief Calculate effective volume for a sound
     * @param baseVolume Base volume of the sound
     * @return Effective volume considering master and category volumes
     */
    float calculateEffectiveSFXVolume(float baseVolume) const;

    /**
     * @brief Calculate effective volume for music
     * @param baseVolume Base volume of the music
     * @return Effective volume considering master and music volumes
     */
    float calculateEffectiveMusicVolume(float baseVolume) const;

    bool                                               m_initialized = false;
    std::vector<SoundSlot>                             m_soundPool;
    std::unordered_map<std::string, sf::SoundBuffer>   m_soundBuffers;
    std::unordered_map<std::string, std::string>       m_musicPaths;  ///< Map music IDs to file paths
    std::unique_ptr<sf::Music>                         m_currentMusic;
    std::string                                        m_currentMusicId;

    float m_masterVolume = AudioConstants::DEFAULT_MASTER_VOLUME;
    float m_sfxVolume    = AudioConstants::DEFAULT_SFX_VOLUME;
    float m_musicVolume  = AudioConstants::DEFAULT_MUSIC_VOLUME;
};

#endif  // SAUDIOSYSTEM_H
