#include "CAudioSource.h"
#include <spdlog/spdlog.h>
#include "CTransform.h"
#include "Entity.h"
#include "SAudioSystem.h"

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
    if (!isActive())
    {
        return;
    }

    // For SFX only: sync position if spatial and playing
    if (m_type == AudioType::SFX && m_spatial && m_playHandle.isValid())
    {
        auto* transform = getOwner()->getComponent<CTransform>();
        if (transform)
        {
            SAudioSystem::instance().setSFXPosition(m_playHandle, transform->getPosition());
        }
    }

    // Check if sound has finished (for non-looping sounds)
    if (m_type == AudioType::SFX && m_playHandle.isValid())
    {
        if (!SAudioSystem::instance().isPlayingSFX(m_playHandle))
        {
            m_playHandle = AudioHandle::invalid();
        }
    }
}

void CAudioSource::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("type");
    builder.addString(getType());
    builder.addKey("data");
    builder.beginObject();

    builder.addKey("guid");
    builder.addString(getGuid());

    builder.addKey("clip");
    builder.addString(m_clipId);

    builder.addKey("audioType");
    builder.addString(m_type == AudioType::SFX ? "sfx" : "music");

    builder.addKey("volume");
    builder.addNumber(m_volume);

    builder.addKey("pitch");
    builder.addNumber(m_pitch);

    builder.addKey("loop");
    builder.addBool(m_loop);

    builder.addKey("spatial");
    builder.addBool(m_spatial);

    builder.addKey("playOnAwake");
    builder.addBool(m_playOnAwake);

    builder.addKey("minDistance");
    builder.addNumber(m_minDistance);

    builder.addKey("attenuation");
    builder.addNumber(m_attenuation);

    builder.endObject();  // data
    builder.endObject();  // component
}

void CAudioSource::deserialize(const JsonValue& value)
{
    if (!value.isObject())
    {
        return;
    }

    const auto& data = value["data"];
    if (!data.isObject())
    {
        return;
    }

    if (data.hasKey("guid"))
    {
        setGuid(data["guid"].getString());
    }

    if (data.hasKey("clip"))
    {
        m_clipId = data["clip"].getString();
    }

    if (data.hasKey("audioType"))
    {
        std::string typeStr = data["audioType"].getString();
        m_type              = (typeStr == "music") ? AudioType::Music : AudioType::SFX;
    }

    if (data.hasKey("volume"))
    {
        m_volume = static_cast<float>(data["volume"].getNumber());
    }

    if (data.hasKey("pitch"))
    {
        m_pitch = static_cast<float>(data["pitch"].getNumber());
    }

    if (data.hasKey("loop"))
    {
        m_loop = data["loop"].getBool();
    }

    if (data.hasKey("spatial"))
    {
        m_spatial = data["spatial"].getBool();
    }

    if (data.hasKey("playOnAwake"))
    {
        m_playOnAwake = data["playOnAwake"].getBool();
    }

    if (data.hasKey("minDistance"))
    {
        m_minDistance = static_cast<float>(data["minDistance"].getNumber());
    }

    if (data.hasKey("attenuation"))
    {
        m_attenuation = static_cast<float>(data["attenuation"].getNumber());
    }
}

bool CAudioSource::play()
{
    if (m_clipId.empty())
    {
        spdlog::warn("CAudioSource: Cannot play, no clip ID set");
        return false;
    }

    auto& audioSystem = SAudioSystem::instance();

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
            auto* transform = getOwner()->getComponent<CTransform>();
            Vec2  position  = transform ? transform->getPosition() : Vec2(0.0f, 0.0f);
            m_playHandle = audioSystem.playSpatialSFX(m_clipId, position, m_volume, m_pitch, m_loop, m_minDistance, m_attenuation);
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
    auto& audioSystem = SAudioSystem::instance();

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
    auto& audioSystem = SAudioSystem::instance();

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
    const auto& audioSystem = SAudioSystem::instance();

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
