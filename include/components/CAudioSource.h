#ifndef CAUDIOSOURCE_H
#define CAUDIOSOURCE_H

#include <string>
#include "AudioTypes.h"
#include "Component.h"

namespace Components
{

/**
 * @brief Component for audio playback on an entity
 *
 * @description
 * CAudioSource allows entities to play sound effects or music. It supports
 * both 2D and spatial (3D) audio playback with configurable parameters.
 * The component automatically manages playback state and can synchronize
 * spatial audio position with the entity's transform.
 *
 * Features:
 * - Play SFX or Music audio types
 * - Spatial audio with attenuation
 * - Volume, pitch, and loop control
 * - Play on awake support
 * - Automatic position synchronization for spatial sounds
 */
class CAudioSource : public Component
{
public:
    CAudioSource();
    ~CAudioSource() override = default;

    void        init() override;
    void        update(float deltaTime) override;
    void        serialize(Serialization::JsonBuilder& builder) const override;
    void        deserialize(const Serialization::SSerialization::JsonValue& value) override;
    std::string getType() const override
    {
        return "AudioSource";
    }

    /**
     * @brief Play the audio clip
     * @return true if playback started successfully
     */
    bool play();

    /**
     * @brief Pause the currently playing audio
     */
    void pause();

    /**
     * @brief Stop the currently playing audio
     */
    void stop();

    /**
     * @brief Check if audio is currently playing
     * @return true if audio is playing
     */
    bool isPlaying() const;

    /**
     * @brief Set the volume of this audio source
     * @param volume Volume level (0.0 to 1.0)
     */
    void setVolume(float volume);

    /**
     * @brief Set the pitch of this audio source
     * @param pitch Pitch multiplier (default 1.0)
     */
    void setPitch(float pitch);

    /**
     * @brief Set whether the audio should loop
     * @param loop true to loop, false to play once
     */
    void setLoop(bool loop);

    /**
     * @brief Set the audio clip ID to play
     * @param clipId Identifier of the loaded audio clip
     */
    void setClipId(const std::string& clipId)
    {
        m_clipId = clipId;
    }

    /**
     * @brief Get the audio clip ID
     * @return The clip identifier
     */
    const std::string& getClipId() const
    {
        return m_clipId;
    }

    /**
     * @brief Set the audio type (SFX or Music)
     * @param type Audio type
     */
    void setAudioType(AudioType type)
    {
        m_type = type;
    }

    /**
     * @brief Get the audio type
     * @return Audio type (SFX or Music)
     */
    AudioType getAudioType() const
    {
        return m_type;
    }

    /**
     * @brief Set whether this is a spatial audio source
     * @param spatial true for 3D spatial audio, false for 2D
     */
    void setSpatial(bool spatial)
    {
        m_spatial = spatial;
    }

    /**
     * @brief Check if this is a spatial audio source
     * @return true if spatial audio is enabled
     */
    bool isSpatial() const
    {
        return m_spatial;
    }

    /**
     * @brief Set whether audio should play automatically on init
     * @param playOnAwake true to auto-play
     */
    void setPlayOnAwake(bool playOnAwake)
    {
        m_playOnAwake = playOnAwake;
    }

    /**
     * @brief Get the current volume
     * @return Volume level (0.0 to 1.0)
     */
    float getVolume() const
    {
        return m_volume;
    }

    /**
     * @brief Get the current pitch
     * @return Pitch multiplier
     */
    float getPitch() const
    {
        return m_pitch;
    }

    /**
     * @brief Check if audio is set to loop
     * @return true if looping is enabled
     */
    bool isLooping() const
    {
        return m_loop;
    }

private:
    std::string m_clipId;                        ///< Audio clip identifier
    AudioType   m_type        = AudioType::SFX;  ///< Type of audio (SFX or Music)
    float       m_volume      = AudioConstants::DEFAULT_SFX_VOLUME;
    float       m_pitch       = AudioConstants::DEFAULT_AUDIO_PITCH;
    bool        m_loop        = false;
    bool        m_spatial     = false;  ///< Enable 3D spatial audio
    bool        m_playOnAwake = false;  ///< Auto-play on initialization
    float       m_minDistance = AudioConstants::DEFAULT_MIN_DISTANCE;
    float       m_attenuation = AudioConstants::DEFAULT_ATTENUATION;
    AudioHandle m_playHandle  = AudioHandle::invalid();  ///< Handle to active sound
};

}  // namespace Components

#endif  // CAUDIOSOURCE_H
