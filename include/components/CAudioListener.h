#ifndef CAUDIOLISTENER_H
#define CAUDIOLISTENER_H

#include <string>
#include <unordered_map>
#include "AudioTypes.h"
#include "Component.h"

namespace Components
{

/**
 * @brief Component for audio listener (typically attached to camera or player)
 *
 * @description
 * CAudioListener represents the "ears" in the audio system. It automatically
 * synchronizes the SFML audio listener position with the entity's transform,
 * enabling proper spatial audio positioning. It also manages a collection of
 * named audio sources that can be triggered by gameplay logic.
 *
 * Features:
 * - Automatic position synchronization with CTransform
 * - Named audio source management (e.g., "jump", "footstep", "ambient")
 * - Default listener tracking (only one should be active)
 * - Convenient play/pause/stop by name interface
 */
class CAudioListener : public Component
{
public:
    /**
     * @brief Configuration for a named audio source
     */
    struct AudioSourceConfig
    {
        std::string clipId;                        ///< Audio clip identifier
        AudioType   type        = AudioType::SFX;  ///< Type of audio
        float       volume      = AudioConstants::DEFAULT_SFX_VOLUME;
        float       pitch       = AudioConstants::DEFAULT_AUDIO_PITCH;
        bool        loop        = false;
        bool        spatial     = false;  ///< Enable 3D spatial audio
        float       minDistance = AudioConstants::DEFAULT_MIN_DISTANCE;
        float       attenuation = AudioConstants::DEFAULT_ATTENUATION;
        AudioHandle playHandle  = AudioHandle::invalid();  ///< Active playback handle (SFX only)
    };

    CAudioListener();
    ~CAudioListener() override = default;

    void        init() override;
    void        update(float deltaTime) override;
    void        serialize(Serialization::JsonBuilder& builder) const override;
    void        deserialize(const Serialization::SSerialization::JsonValue& value) override;
    std::string getType() const override
    {
        return "AudioListener";
    }

    /**
     * @brief Set whether this is the default (active) listener
     * @param isDefault true to make this the default listener
     */
    void setDefaultListener(bool isDefault)
    {
        m_isDefaultListener = isDefault;
    }

    /**
     * @brief Check if this is the default listener
     * @return true if this is the default listener
     */
    bool isDefaultListener() const
    {
        return m_isDefaultListener;
    }

    /**
     * @brief Add a named audio source configuration
     * @param name Unique name for this audio source
     * @param config Audio source configuration
     */
    void addAudioSource(const std::string& name, const AudioSourceConfig& config);

    /**
     * @brief Remove a named audio source
     * @param name Name of the audio source to remove
     */
    void removeAudioSource(const std::string& name);

    /**
     * @brief Play a named audio source
     * @param name Name of the audio source to play
     * @return true if playback started successfully
     */
    bool play(const std::string& name);

    /**
     * @brief Pause a named audio source
     * @param name Name of the audio source to pause
     */
    void pause(const std::string& name);

    /**
     * @brief Stop a named audio source
     * @param name Name of the audio source to stop
     */
    void stop(const std::string& name);

    /**
     * @brief Check if a named audio source is playing
     * @param name Name of the audio source to check
     * @return true if the audio is playing
     */
    bool isPlaying(const std::string& name) const;

    /**
     * @brief Get all named audio sources
     * @return Map of audio source configurations
     */
    const std::unordered_map<std::string, AudioSourceConfig>& getAudioSources() const
    {
        return m_audioSources;
    }

private:
    bool m_isDefaultListener = true;                                    ///< Whether this is the active listener
    std::unordered_map<std::string, AudioSourceConfig> m_audioSources;  ///< Named audio sources
};

}  // namespace Components

#endif  // CAUDIOLISTENER_H
