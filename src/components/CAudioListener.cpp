#include "CAudioListener.h"
#include <spdlog/spdlog.h>

namespace Components
{

CAudioListener::CAudioListener() {}

void CAudioListener::init()
{
    // Nothing to do on init; ECS audio system will process listener state.
}

void CAudioListener::update(float deltaTime)
{
    (void)deltaTime;
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
        it->second.playHandle     = AudioHandle::invalid();
        it->second.pendingCommand = AudioCommand::Stop;
        it->second.isPlaying      = false;
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

    if (config.clipId.empty())
    {
        spdlog::warn("CAudioListener: Audio source '{}' has no clip ID", name);
        return false;
    }

    config.pendingCommand = AudioCommand::Play;
    return true;
}

void CAudioListener::pause(const std::string& name)
{
    auto it = m_audioSources.find(name);
    if (it == m_audioSources.end())
    {
        return;
    }

    auto& config      = it->second;
    config.pendingCommand = AudioCommand::Pause;
}

void CAudioListener::stop(const std::string& name)
{
    auto it = m_audioSources.find(name);
    if (it == m_audioSources.end())
    {
        return;
    }

    auto& config          = it->second;
    config.pendingCommand = AudioCommand::Stop;
    config.playHandle     = AudioHandle::invalid();
    config.isPlaying      = false;
}

bool CAudioListener::isPlaying(const std::string& name) const
{
    auto it = m_audioSources.find(name);
    if (it == m_audioSources.end())
    {
        return false;
    }

    const auto& config = it->second;
    return config.isPlaying;
}

}  // namespace Components
