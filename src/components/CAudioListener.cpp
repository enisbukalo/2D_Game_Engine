#include "CAudioListener.h"
#include <spdlog/spdlog.h>
#include "CTransform.h"
// #include "Entity.h" // Removed - Entity is now just an ID
#include "SAudio.h"
#include "SystemLocator.h"

namespace Components
{

CAudioListener::CAudioListener() {}

void CAudioListener::init()
{
#if 0  // TODO: Requires Registry access - needs refactoring
    // Set listener position on initialization
    if (m_isDefaultListener)
    {
        auto* transform = getOwner()->getComponent<CTransform>();
        if (transform)
        {
            ::Systems::SystemLocator::audio().setListenerPosition(transform->getPosition());
        }
    }
#endif
}

void CAudioListener::update(float deltaTime)
{
    if (!m_isDefaultListener)
    {
        return;
    }

#if 0  // TODO: Requires Registry access in update()
    // Synchronize listener position with transform
    auto* transform = registry.tryGet<CTransform>(getOwner());
    if (transform)
    {
        ::Systems::SystemLocator::audio().setListenerPosition(transform->getPosition());
    }
#endif

    // Update spatial audio positions for named sources
    for (auto& [name, config] : m_audioSources)
    {
        if (config.type == AudioType::SFX && config.spatial && config.playHandle.isValid())
        {
            // Check if sound is still playing
            if (!::Systems::SystemLocator::audio().isPlayingSFX(config.playHandle))
            {
                config.playHandle = AudioHandle::invalid();
            }
        }
    }
}

void CAudioListener::addAudioSource(const std::string& name, const AudioSourceConfig& config)
{
    m_audioSources[name] = config;
}

void CAudioListener::removeAudioSource(const std::string& name)
{
    auto it = m_audioSources.find(name);
    if (it != m_audioSources.end())
    {
        // Stop if playing
        if (it->second.type == AudioType::SFX && it->second.playHandle.isValid())
        {
            ::Systems::SystemLocator::audio().stopSFX(it->second.playHandle);
        }
        m_audioSources.erase(it);
    }
}

bool CAudioListener::play(const std::string& name)
{
    auto it = m_audioSources.find(name);
    if (it == m_audioSources.end())
    {
        spdlog::warn("CAudioListener: Audio source '{}' not found", name);
        return false;
    }

    auto& config      = it->second;
    auto& audioSystem = ::Systems::SystemLocator::audio();

    if (config.clipId.empty())
    {
        spdlog::warn("CAudioListener: Audio source '{}' has no clip ID", name);
        return false;
    }

    if (config.type == AudioType::SFX)
    {
        // Stop current sound if playing
        if (config.playHandle.isValid())
        {
            audioSystem.stopSFX(config.playHandle);
        }

        // Play spatial or non-spatial SFX
        if (config.spatial)
        {
#if 0  // TODO: Requires Registry access
            auto* transform = registry.tryGet<CTransform>(getOwner());
            Vec2  position    = transform ? transform->getPosition() : Vec2(0.0f, 0.0f);
            config.playHandle = audioSystem.playSpatialSFX(
                config.clipId, position, config.volume, config.pitch, config.loop, config.minDistance, config.attenuation);
#else
            // Temporary: Play non-spatial until Registry is added to component methods
            config.playHandle = audioSystem.playSFX(config.clipId, config.volume, config.pitch, config.loop);
#endif
        }
        else
        {
            config.playHandle = audioSystem.playSFX(config.clipId, config.volume, config.pitch, config.loop);
        }

        return config.playHandle.isValid();
    }
    else  // AudioType::Music
    {
        return audioSystem.playMusic(config.clipId, config.loop, config.volume);
    }
}

void CAudioListener::pause(const std::string& name)
{
    auto it = m_audioSources.find(name);
    if (it == m_audioSources.end())
    {
        return;
    }

    auto& config      = it->second;
    auto& audioSystem = ::Systems::SystemLocator::audio();

    if (config.type == AudioType::SFX && config.playHandle.isValid())
    {
        audioSystem.pauseSFX(config.playHandle);
    }
    else if (config.type == AudioType::Music)
    {
        audioSystem.pauseMusic();
    }
}

void CAudioListener::stop(const std::string& name)
{
    auto it = m_audioSources.find(name);
    if (it == m_audioSources.end())
    {
        return;
    }

    auto& config      = it->second;
    auto& audioSystem = ::Systems::SystemLocator::audio();

    if (config.type == AudioType::SFX && config.playHandle.isValid())
    {
        audioSystem.stopSFX(config.playHandle);
        config.playHandle = AudioHandle::invalid();
    }
    else if (config.type == AudioType::Music)
    {
        audioSystem.stopMusic();
    }
}

bool CAudioListener::isPlaying(const std::string& name) const
{
    auto it = m_audioSources.find(name);
    if (it == m_audioSources.end())
    {
        return false;
    }

    const auto& config      = it->second;
    const auto& audioSystem = ::Systems::SystemLocator::audio();

    if (config.type == AudioType::SFX)
    {
        return config.playHandle.isValid() && audioSystem.isPlayingSFX(config.playHandle);
    }
    else  // AudioType::Music
    {
        return audioSystem.isMusicPlaying();
    }
}

}  // namespace Components
