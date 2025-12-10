#include "CAudioListener.h"
#include <spdlog/spdlog.h>
#include "CTransform.h"
// #include "Entity.h" // Removed - Entity is now just an ID
#include "SAudio.h"

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
            ::Systems::SAudio::instance().setListenerPosition(transform->getPosition());
        }
    }
#endif
}

void CAudioListener::update(float deltaTime)
{
    if (!isActive() || !m_isDefaultListener)
    {
        return;
    }

#if 0  // TODO: Requires Registry access in update()
    // Synchronize listener position with transform
    auto* transform = registry.tryGet<CTransform>(getOwner());
    if (transform)
    {
        ::Systems::SAudio::instance().setListenerPosition(transform->getPosition());
    }
#endif

    // Update spatial audio positions for named sources
    for (auto& [name, config] : m_audioSources)
    {
        if (config.type == AudioType::SFX && config.spatial && config.playHandle.isValid())
        {
            // Check if sound is still playing
            if (!::Systems::SAudio::instance().isPlayingSFX(config.playHandle))
            {
                config.playHandle = AudioHandle::invalid();
            }
        }
    }
}

void CAudioListener::serialize(Serialization::JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("type");
    builder.addString(getType());
    builder.addKey("data");
    builder.beginObject();

    builder.addKey("guid");
    builder.addString(getGuid());

    builder.addKey("isDefaultListener");
    builder.addBool(m_isDefaultListener);

    // Serialize named audio sources
    builder.addKey("audioSources");
    builder.beginArray();
    for (const auto& [name, config] : m_audioSources)
    {
        builder.beginObject();

        builder.addKey("name");
        builder.addString(name);

        builder.addKey("clip");
        builder.addString(config.clipId);

        builder.addKey("audioType");
        builder.addString(config.type == AudioType::SFX ? "sfx" : "music");

        builder.addKey("volume");
        builder.addNumber(config.volume);

        builder.addKey("pitch");
        builder.addNumber(config.pitch);

        builder.addKey("loop");
        builder.addBool(config.loop);

        builder.addKey("spatial");
        builder.addBool(config.spatial);

        builder.addKey("minDistance");
        builder.addNumber(config.minDistance);

        builder.addKey("attenuation");
        builder.addNumber(config.attenuation);

        builder.endObject();
    }
    builder.endArray();

    builder.endObject();  // data
    builder.endObject();  // component
}

void CAudioListener::deserialize(const Serialization::SSerialization::JsonValue& value)
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

    if (data.hasKey("isDefaultListener"))
    {
        m_isDefaultListener = data["isDefaultListener"].getBool();
    }

    // Deserialize named audio sources
    if (data.hasKey("audioSources") && data["audioSources"].isArray())
    {
        const auto& sources = data["audioSources"].getArray();
        for (const auto& sourceValue : sources)
        {
            if (!sourceValue.isObject())
            {
                continue;
            }

            AudioSourceConfig config;
            std::string       name;

            if (sourceValue.hasKey("name"))
            {
                name = sourceValue["name"].getString();
            }

            if (sourceValue.hasKey("clip"))
            {
                config.clipId = sourceValue["clip"].getString();
            }

            if (sourceValue.hasKey("audioType"))
            {
                std::string typeStr = sourceValue["audioType"].getString();
                config.type         = (typeStr == "music") ? AudioType::Music : AudioType::SFX;
            }

            if (sourceValue.hasKey("volume"))
            {
                config.volume = static_cast<float>(sourceValue["volume"].getNumber());
            }

            if (sourceValue.hasKey("pitch"))
            {
                config.pitch = static_cast<float>(sourceValue["pitch"].getNumber());
            }

            if (sourceValue.hasKey("loop"))
            {
                config.loop = sourceValue["loop"].getBool();
            }

            if (sourceValue.hasKey("spatial"))
            {
                config.spatial = sourceValue["spatial"].getBool();
            }

            if (sourceValue.hasKey("minDistance"))
            {
                config.minDistance = static_cast<float>(sourceValue["minDistance"].getNumber());
            }

            if (sourceValue.hasKey("attenuation"))
            {
                config.attenuation = static_cast<float>(sourceValue["attenuation"].getNumber());
            }

            if (!name.empty())
            {
                m_audioSources[name] = config;
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
            ::Systems::SAudio::instance().stopSFX(it->second.playHandle);
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
    auto& audioSystem = ::Systems::SAudio::instance();

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
    auto& audioSystem = ::Systems::SAudio::instance();

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
    auto& audioSystem = ::Systems::SAudio::instance();

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
    const auto& audioSystem = ::Systems::SAudio::instance();

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
