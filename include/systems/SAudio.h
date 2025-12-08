#ifndef SAUDIO_H
#define SAUDIO_H

#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "IAudioSystem.h"

namespace Systems
{

/**
 * @brief SFML-based implementation of the audio system
 *
 * @description
 * SAudio implements audio playback using SFML's audio module.
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
class SAudio : public IAudioSystem
{
public:
    /**
     * @brief Construct audio system with specified pool size
     * @param poolSize Number of simultaneous sound effects (default 32)
     */
    explicit SAudio(size_t poolSize = AudioConstants::DEFAULT_SFX_POOL_SIZE);

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~SAudio() override;

    // IAudioSystem interface implementation
    bool initialize() override;
    void shutdown() override;

    bool loadSound(const std::string& id, const std::string& filepath, AudioType type) override;
    void unloadSound(const std::string& id) override;

    AudioHandle playSFX(const std::string& id, float volume = 1.0f, float pitch = 1.0f, bool loop = false) override;
    AudioHandle playSpatialSFX(const std::string& id,
                               const Vec2&        position,
                               float              volume      = 1.0f,
                               float              pitch       = 1.0f,
                               bool               loop        = false,
                               float              minDistance = AudioConstants::DEFAULT_MIN_DISTANCE,
                               float              attenuation = AudioConstants::DEFAULT_ATTENUATION) override;

    void stopSFX(AudioHandle handle) override;
    void pauseSFX(AudioHandle handle) override;
    void resumeSFX(AudioHandle handle) override;
    bool isPlayingSFX(AudioHandle handle) const override;
    void setSFXPosition(AudioHandle handle, const Vec2& position) override;
    void setSFXVolume(AudioHandle handle, float volume) override;

    /**
     * @brief Play a sound effect with optional fade-in
     * @param id Sound ID to play
     * @param volume Base volume (0.0 to 1.0)
     * @param pitch Pitch multiplier
     * @param loop Whether to loop the sound
     * @param fadeConfig Fade configuration (default: instant)
     * @return Handle to the playing sound
     */
    AudioHandle playSFXWithFade(const std::string& id,
                                float              volume     = 1.0f,
                                float              pitch      = 1.0f,
                                bool               loop       = false,
                                const FadeConfig&  fadeConfig = FadeConfig::instant());

    /**
     * @brief Play a spatial sound effect with optional fade-in
     * @param id Sound ID to play
     * @param position World position
     * @param volume Base volume (0.0 to 1.0)
     * @param pitch Pitch multiplier
     * @param loop Whether to loop the sound
     * @param minDistance Minimum distance for attenuation
     * @param attenuation Attenuation factor
     * @param fadeConfig Fade configuration (default: instant)
     * @return Handle to the playing sound
     */
    AudioHandle playSpatialSFXWithFade(const std::string& id,
                                       const Vec2&        position,
                                       float              volume      = 1.0f,
                                       float              pitch       = 1.0f,
                                       bool               loop        = false,
                                       float              minDistance = AudioConstants::DEFAULT_MIN_DISTANCE,
                                       float              attenuation = AudioConstants::DEFAULT_ATTENUATION,
                                       const FadeConfig&  fadeConfig  = FadeConfig::instant());

    /**
     * @brief Fade a playing sound to a target volume
     * @param handle Handle to the sound
     * @param targetVolume Target volume (0.0 to 1.0)
     * @param fadeConfig Fade configuration
     * @return true if fade was started, false if handle invalid or fade rejected
     */
    bool fadeSFX(AudioHandle handle, float targetVolume, const FadeConfig& fadeConfig);

    /**
     * @brief Stop a sound with optional fade-out
     * @param handle Handle to the sound
     * @param fadeConfig Fade configuration (default: instant stop)
     */
    void stopSFXWithFade(AudioHandle handle, const FadeConfig& fadeConfig = FadeConfig::instant());

    /**
     * @brief Cancel any active fade on a sound
     * @param handle Handle to the sound
     */
    void cancelFade(AudioHandle handle);

    bool playMusic(const std::string& id, bool loop = true, float volume = 1.0f) override;
    void stopMusic() override;
    void pauseMusic() override;
    void resumeMusic() override;
    bool isMusicPlaying() const override;

    /**
     * @brief Play music with optional fade-in
     * @param id Music ID to play
     * @param loop Whether to loop the music
     * @param volume Base volume (0.0 to 1.0)
     * @param fadeConfig Fade configuration (default: instant)
     * @return true if music started successfully
     */
    bool playMusicWithFade(const std::string& id, bool loop = true, float volume = 1.0f, const FadeConfig& fadeConfig = FadeConfig::instant());

    /**
     * @brief Fade music to a target volume
     * @param targetVolume Target volume (0.0 to 1.0)
     * @param fadeConfig Fade configuration
     * @return true if fade was started, false if no music playing or fade rejected
     */
    bool fadeMusic(float targetVolume, const FadeConfig& fadeConfig);

    /**
     * @brief Stop music with optional fade-out
     * @param fadeConfig Fade configuration (default: instant stop)
     */
    void stopMusicWithFade(const FadeConfig& fadeConfig = FadeConfig::instant());

    /**
     * @brief Cancel any active fade on music
     */
    void cancelMusicFade();

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
    static SAudio& instance();

    // Delete copy and move constructors/assignment operators
    SAudio(const SAudio&)            = delete;
    SAudio(SAudio&&)                 = delete;
    SAudio& operator=(const SAudio&) = delete;
    SAudio& operator=(SAudio&&)      = delete;

private:
    /**
     * @brief Sound pool slot with generation tracking and fade state
     */
    struct SoundSlot
    {
        sf::Sound sound;
        uint32_t  generation = 0;
        bool      inUse      = false;
        float     baseVolume = 1.0f;  // Store base volume before master/category multipliers

        // Fade state tracking
        FadeState             fadeState      = FadeState::None;
        float                 targetVolume   = 1.0f;  ///< Target volume for fade
        float                 startVolume    = 1.0f;  ///< Starting volume for fade
        float                 fadeDuration   = 0.0f;  ///< Total fade duration
        float                 fadeElapsed    = 0.0f;  ///< Time elapsed in current fade
        FadeCurve             fadeCurve      = FadeCurve::Linear;
        bool                  allowInterrupt = true;     ///< Can this fade be interrupted
        std::function<void()> onFadeComplete = nullptr;  ///< Callback when fade completes
        bool                  stopAfterFade  = false;    ///< Stop the sound after fade completes
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

    /**
     * @brief Apply fade curve interpolation
     * @param t Normalized time (0.0 to 1.0)
     * @param curve Curve type to apply
     * @return Interpolated value (0.0 to 1.0)
     */
    float applyFadeCurve(float t, FadeCurve curve) const;

    /**
     * @brief Update fade state for a sound slot
     * @param slot Sound slot to update
     * @param deltaTime Time since last update
     */
    void updateSoundFade(SoundSlot& slot, float deltaTime);

    /**
     * @brief Update fade state for music
     * @param deltaTime Time since last update
     */
    void updateMusicFade(float deltaTime);

    bool                                             m_initialized = false;
    std::vector<SoundSlot>                           m_soundPool;
    std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers;
    std::unordered_map<std::string, std::string>     m_musicPaths;  ///< Map music IDs to file paths
    std::unique_ptr<sf::Music>                       m_currentMusic;
    std::string                                      m_currentMusicId;
    float m_currentMusicBaseVolume = 1.0f;  ///< Base volume for current music

    // Music fade state
    FadeState             m_musicFadeState      = FadeState::None;
    float                 m_musicTargetVolume   = 1.0f;
    float                 m_musicStartVolume    = 1.0f;
    float                 m_musicFadeDuration   = 0.0f;
    float                 m_musicFadeElapsed    = 0.0f;
    FadeCurve             m_musicFadeCurve      = FadeCurve::Linear;
    bool                  m_musicAllowInterrupt = true;
    std::function<void()> m_musicOnFadeComplete = nullptr;
    bool                  m_musicStopAfterFade  = false;

    float m_masterVolume = AudioConstants::DEFAULT_MASTER_VOLUME;
    float m_sfxVolume    = AudioConstants::DEFAULT_SFX_VOLUME;
    float m_musicVolume  = AudioConstants::DEFAULT_MUSIC_VOLUME;
};

}  // namespace Systems

#endif  // SAUDIO_H
