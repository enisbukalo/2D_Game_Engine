#ifndef IAUDIOSYSTEM_H
#define IAUDIOSYSTEM_H

#include <string>
#include "AudioTypes.h"
#include "Vec2.h"

/**
 * @brief Abstract interface for audio system operations
 *
 * @description
 * IAudioSystem provides a platform-agnostic interface for audio playback.
 * Implementations handle sound effects (pooled, in-memory) and music (streamed).
 * All methods should be called from the main thread unless documented otherwise.
 *
 * Audio System Features:
 * - SFX: Fixed-size sound pool with in-memory buffers for fast playback
 * - Music: Streamed playback for long-form audio (single active track)
 * - Volume control: Master, SFX, and Music volume categories
 * - Spatial audio: 3D positioning for sound effects with attenuation
 */
class IAudioSystem
{
public:
    virtual ~IAudioSystem() = default;

    /**
     * @brief Initialize the audio system
     * @return true if initialization succeeded
     */
    virtual bool initialize() = 0;

    /**
     * @brief Shutdown the audio system and release resources
     */
    virtual void shutdown() = 0;

    /**
     * @brief Load an audio resource into memory or prepare for streaming
     * @param id Unique identifier for this audio resource
     * @param filepath Path to the audio file
     * @param type Type of audio (SFX or Music)
     * @return true if loading succeeded
     */
    virtual bool loadSound(const std::string& id, const std::string& filepath, AudioType type) = 0;

    /**
     * @brief Unload an audio resource and free memory
     * @param id Identifier of the resource to unload
     */
    virtual void unloadSound(const std::string& id) = 0;

    /**
     * @brief Play a sound effect
     * @param id Identifier of the loaded sound
     * @param volume Volume multiplier (0.0 to 1.0)
     * @param pitch Pitch multiplier (default 1.0)
     * @param loop Whether to loop the sound
     * @return Handle to the playing sound, or invalid handle if failed
     */
    virtual AudioHandle playSFX(const std::string& id, float volume = 1.0f, float pitch = 1.0f, bool loop = false) = 0;

    /**
     * @brief Play a spatial sound effect at a 3D position
     * @param id Identifier of the loaded sound
     * @param position 2D position in world space
     * @param volume Volume multiplier (0.0 to 1.0)
     * @param pitch Pitch multiplier (default 1.0)
     * @param loop Whether to loop the sound
     * @param minDistance Distance at which attenuation begins
     * @param attenuation Attenuation factor
     * @return Handle to the playing sound, or invalid handle if failed
     */
    virtual AudioHandle playSpatialSFX(const std::string& id,
                                       const Vec2&        position,
                                       float              volume      = 1.0f,
                                       float              pitch       = 1.0f,
                                       bool               loop        = false,
                                       float              minDistance = AudioConstants::DEFAULT_MIN_DISTANCE,
                                       float              attenuation = AudioConstants::DEFAULT_ATTENUATION) = 0;

    /**
     * @brief Stop a playing sound effect
     * @param handle Handle to the sound to stop
     */
    virtual void stopSFX(AudioHandle handle) = 0;

    /**
     * @brief Pause a playing sound effect
     * @param handle Handle to the sound to pause
     */
    virtual void pauseSFX(AudioHandle handle) = 0;

    /**
     * @brief Resume a paused sound effect
     * @param handle Handle to the sound to resume
     */
    virtual void resumeSFX(AudioHandle handle) = 0;

    /**
     * @brief Check if a sound effect is currently playing
     * @param handle Handle to check
     * @return true if the sound is playing
     */
    virtual bool isPlayingSFX(AudioHandle handle) const = 0;

    /**
     * @brief Update the position of a spatial sound effect
     * @param handle Handle to the sound
     * @param position New position in world space
     */
    virtual void setSFXPosition(AudioHandle handle, const Vec2& position) = 0;

    /**
     * @brief Set the volume of a specific sound effect
     * @param handle Handle to the sound
     * @param volume Volume multiplier (0.0 to 1.0)
     */
    virtual void setSFXVolume(AudioHandle handle, float volume) = 0;

    /**
     * @brief Play background music
     * @param id Identifier of the loaded music
     * @param loop Whether to loop the music
     * @param volume Volume multiplier (0.0 to 1.0)
     * @return true if music playback started successfully
     */
    virtual bool playMusic(const std::string& id, bool loop = true, float volume = 1.0f) = 0;

    /**
     * @brief Stop the currently playing music
     */
    virtual void stopMusic() = 0;

    /**
     * @brief Pause the currently playing music
     */
    virtual void pauseMusic() = 0;

    /**
     * @brief Resume paused music
     */
    virtual void resumeMusic() = 0;

    /**
     * @brief Check if music is currently playing
     * @return true if music is playing
     */
    virtual bool isMusicPlaying() const = 0;

    /**
     * @brief Set master volume (affects all audio)
     * @param volume Volume level (0.0 to 1.0)
     */
    virtual void setMasterVolume(float volume) = 0;

    /**
     * @brief Set sound effects volume
     * @param volume Volume level (0.0 to 1.0)
     */
    virtual void setSFXVolume(float volume) = 0;

    /**
     * @brief Set music volume
     * @param volume Volume level (0.0 to 1.0)
     */
    virtual void setMusicVolume(float volume) = 0;

    /**
     * @brief Get current master volume
     * @return Volume level (0.0 to 1.0)
     */
    virtual float getMasterVolume() const = 0;

    /**
     * @brief Get current SFX volume
     * @return Volume level (0.0 to 1.0)
     */
    virtual float getSFXVolume() const = 0;

    /**
     * @brief Get current music volume
     * @return Volume level (0.0 to 1.0)
     */
    virtual float getMusicVolume() const = 0;

    /**
     * @brief Set the position of the audio listener (typically the camera/player)
     * @param position Listener position in world space
     */
    virtual void setListenerPosition(const Vec2& position) = 0;

    /**
     * @brief Update the audio system (called each frame)
     * @param deltaTime Time elapsed since last update
     *
     * This method updates sound states, frees finished sounds from the pool,
     * and performs any necessary audio system bookkeeping.
     */
    virtual void update(float deltaTime) = 0;
};

#endif  // IAUDIOSYSTEM_H
