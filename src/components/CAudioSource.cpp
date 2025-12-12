#include "CAudioSource.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace Components
{

CAudioSource::CAudioSource() {}

void CAudioSource::init()
{
    if (m_playOnAwake && !m_playQueued)
    {
        m_pendingCommand = AudioCommand::Play;
        m_playQueued      = true;
    }
}

void CAudioSource::update(float deltaTime)
{
    (void)deltaTime;

    // If play-on-awake was requested but init not called, queue it here lazily.
    if (m_playOnAwake && !m_playQueued)
    {
        m_pendingCommand = AudioCommand::Play;
        m_playQueued      = true;
    }
}

bool CAudioSource::play()
{
    if (m_clipId.empty())
    {
        spdlog::warn("CAudioSource: Cannot play, no clip ID set");
        return false;
    }

    m_pendingCommand = AudioCommand::Play;
    return true;
}

void CAudioSource::pause()
{
    m_pendingCommand = AudioCommand::Pause;
}

void CAudioSource::stop()
{
    m_pendingCommand = AudioCommand::Stop;
}

bool CAudioSource::isPlaying() const
{
    return m_isPlaying;
}

void CAudioSource::setVolume(float volume)
{
    m_volume = std::clamp(volume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);
}

void CAudioSource::setPitch(float pitch)
{
    m_pitch = pitch;
}

void CAudioSource::setLoop(bool loop)
{
    m_loop = loop;
}

}  // namespace Components
