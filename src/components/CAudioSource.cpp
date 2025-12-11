#include "CAudioSource.h"
#include <spdlog/spdlog.h>
#include "CTransform.h"
// #include "Entity.h" // Removed - Entity is now just an ID
#include "SAudio.h"
#include "SystemLocator.h"

namespace Components
{

CAudioSource::CAudioSource() {}

void CAudioSource::init()
{
    if (m_playOnAwake && !m_clipId.empty())
    {
        play();
    }
}

void CAudioSource::update(float deltaTime)
{
#if 0  // TODO: Requires Registry access in update()
    // For SFX only: sync position if spatial and playing
    if (m_type == AudioType::SFX && m_spatial && m_playHandle.isValid())
    {
        auto* transform = registry.tryGet<CTransform>(getOwner());
        if (transform)
        {
            ::Systems::SystemLocator::audio().setSFXPosition(m_playHandle, transform->getPosition());
        }
    }
#endif

    // Check if sound has finished (for non-looping sounds)
    if (m_type == AudioType::SFX && m_playHandle.isValid())
    {
        if (!::Systems::SystemLocator::audio().isPlayingSFX(m_playHandle))
        {
            m_playHandle = AudioHandle::invalid();
        }
    }
}

bool CAudioSource::play()
{
    if (m_clipId.empty())
    {
        spdlog::warn("CAudioSource: Cannot play, no clip ID set");
        return false;
    }

    auto& audioSystem = ::Systems::SystemLocator::audio();

    if (m_type == AudioType::SFX)
    {
        // Stop current sound if playing
        if (m_playHandle.isValid())
        {
            audioSystem.stopSFX(m_playHandle);
        }

        // Play spatial or non-spatial SFX
        if (m_spatial)
        {
#if 0  // TODO: Requires Registry access
            auto* transform = registry.tryGet<CTransform>(getOwner());
            Vec2  position  = transform ? transform->getPosition() : Vec2(0.0f, 0.0f);
            m_playHandle = audioSystem.playSpatialSFX(m_clipId, position, m_volume, m_pitch, m_loop, m_minDistance, m_attenuation);
#else
            // Temporary: Play non-spatial until Registry is added
            m_playHandle = audioSystem.playSFX(m_clipId, m_volume, m_pitch, m_loop);
#endif
        }
        else
        {
            m_playHandle = audioSystem.playSFX(m_clipId, m_volume, m_pitch, m_loop);
        }

        return m_playHandle.isValid();
    }
    else  // AudioType::Music
    {
        return audioSystem.playMusic(m_clipId, m_loop, m_volume);
    }
}

void CAudioSource::pause()
{
    auto& audioSystem = ::Systems::SystemLocator::audio();

    if (m_type == AudioType::SFX && m_playHandle.isValid())
    {
        audioSystem.pauseSFX(m_playHandle);
    }
    else if (m_type == AudioType::Music)
    {
        audioSystem.pauseMusic();
    }
}

void CAudioSource::stop()
{
    auto& audioSystem = ::Systems::SystemLocator::audio();

    if (m_type == AudioType::SFX && m_playHandle.isValid())
    {
        audioSystem.stopSFX(m_playHandle);
        m_playHandle = AudioHandle::invalid();
    }
    else if (m_type == AudioType::Music)
    {
        audioSystem.stopMusic();
    }
}

bool CAudioSource::isPlaying() const
{
    const auto& audioSystem = ::Systems::SystemLocator::audio();

    if (m_type == AudioType::SFX)
    {
        return m_playHandle.isValid() && audioSystem.isPlayingSFX(m_playHandle);
    }
    else  // AudioType::Music
    {
        return audioSystem.isMusicPlaying();
    }
}

void CAudioSource::setVolume(float volume)
{
    m_volume = std::clamp(volume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);

    // Note: Volume changes won't affect already-playing sounds in this implementation
    // To support runtime volume changes, we'd need to track and update active sounds
}

void CAudioSource::setPitch(float pitch)
{
    m_pitch = pitch;

    // Note: Pitch changes won't affect already-playing sounds in this implementation
}

void CAudioSource::setLoop(bool loop)
{
    m_loop = loop;

    // Note: Loop changes won't affect already-playing sounds in this implementation
}

}  // namespace Components
