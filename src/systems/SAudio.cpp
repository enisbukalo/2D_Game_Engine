#include "SAudio.h"
#include <spdlog/spdlog.h>
#include <SFML/Audio.hpp>
#include <algorithm>

#include "CAudioListener.h"
#include "CAudioSource.h"
#include "CTransform.h"
#include "World.h"

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

namespace Systems
{

SAudio::SAudio(size_t poolSize) : m_soundPool(poolSize) {}

SAudio::~SAudio()
{
    // Call shutdown directly without virtual dispatch
    SAudio::shutdown();
}

bool SAudio::initialize()
{
    if (m_initialized)
    {
        spdlog::warn("SAudio already initialized");
        return true;
    }

    spdlog::info("Initializing SFML audio system with pool size: {}", m_soundPool.size());

    // Suppress ALSA/OpenAL errors in headless environments
#ifndef _WIN32
    int oldStderr = -1;
    int devNull   = open("/dev/null", O_WRONLY);
    if (devNull != -1)
    {
        oldStderr = dup(STDERR_FILENO);
        dup2(devNull, STDERR_FILENO);
        close(devNull);
    }
#endif

    // Initialize all sound slots
    for (auto& slot : m_soundPool)
    {
        slot.generation = 0;
        slot.inUse      = false;
    }

    // Set default listener position (may fail silently in headless)
    sf::Listener::setPosition(0.0f, 0.0f, 0.0f);
    sf::Listener::setDirection(0.0f, 0.0f, -1.0f);
    sf::Listener::setUpVector(0.0f, 1.0f, 0.0f);

    // Restore stderr
#ifndef _WIN32
    if (oldStderr != -1)
    {
        dup2(oldStderr, STDERR_FILENO);
        close(oldStderr);
    }
#endif

    m_initialized = true;
    spdlog::info("Audio system initialized successfully");
    return true;
}

void SAudio::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    spdlog::info("Shutting down audio system");

    // Stop all active sounds
    for (auto& slot : m_soundPool)
    {
        if (slot.inUse)
        {
            slot.sound.stop();
            slot.inUse = false;
        }
    }

    // Stop music
    if (m_currentMusic)
    {
        m_currentMusic->stop();
        m_currentMusic.reset();
    }

    // Clear all buffers
    m_soundBuffers.clear();
    m_musicPaths.clear();
    m_currentMusicId.clear();

    m_initialized = false;
    spdlog::info("Audio system shut down");
}

bool SAudio::loadSound(const std::string& id, const std::string& filepath, AudioType type)
{
    if (!m_initialized)
    {
        spdlog::error("Cannot load sound: audio system not initialized");
        return false;
    }

    if (type == AudioType::SFX)
    {
        // Check if already loaded
        if (m_soundBuffers.find(id) != m_soundBuffers.end())
        {
            spdlog::warn("Sound buffer '{}' already loaded", id);
            return true;
        }

        // Suppress ALSA errors during loading in headless environment
#ifndef _WIN32
        int oldStderr = -1;
        int devNull   = open("/dev/null", O_WRONLY);
        if (devNull != -1)
        {
            oldStderr = dup(STDERR_FILENO);
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }
#endif

        sf::SoundBuffer buffer;
        bool            success = buffer.loadFromFile(filepath);

        // Restore stderr
#ifndef _WIN32
        if (oldStderr != -1)
        {
            dup2(oldStderr, STDERR_FILENO);
            close(oldStderr);
        }
#endif

        if (!success)
        {
            spdlog::error("Failed to load sound buffer from file: {}", filepath);
            return false;
        }

        m_soundBuffers[id] = std::move(buffer);
        spdlog::info("Loaded SFX '{}' from '{}'", id, filepath);
        return true;
    }
    else  // AudioType::Music
    {
        // For music, we just store the path for later streaming
        m_musicPaths[id] = filepath;
        spdlog::info("Registered music '{}' with path '{}'", id, filepath);
        return true;
    }
}

void SAudio::unloadSound(const std::string& id)
{
    // Try to unload SFX buffer
    auto bufferIt = m_soundBuffers.find(id);
    if (bufferIt != m_soundBuffers.end())
    {
        // Stop all sounds using this buffer
        for (auto& slot : m_soundPool)
        {
            if (slot.inUse && slot.sound.getBuffer() == &bufferIt->second)
            {
                slot.sound.stop();
                slot.inUse = false;
            }
        }
        m_soundBuffers.erase(bufferIt);
        spdlog::info("Unloaded SFX buffer '{}'", id);
        return;
    }

    // Try to unload music path
    auto musicIt = m_musicPaths.find(id);
    if (musicIt != m_musicPaths.end())
    {
        if (m_currentMusicId == id)
        {
            stopMusic();
        }
        m_musicPaths.erase(musicIt);
        spdlog::info("Unloaded music path '{}'", id);
    }
}

AudioHandle SAudio::playSFX(const std::string& id, float volume, float pitch, bool loop)
{
    if (!m_initialized)
    {
        spdlog::error("Cannot play SFX: audio system not initialized");
        return AudioHandle::invalid();
    }

    // Find the sound buffer
    auto it = m_soundBuffers.find(id);
    if (it == m_soundBuffers.end())
    {
        spdlog::error("Sound buffer '{}' not found", id);
        return AudioHandle::invalid();
    }

    // Find available slot
    int slotIndex = findAvailableSlot();
    if (slotIndex < 0)
    {
        spdlog::warn("Sound pool full, cannot play '{}'", id);
        return AudioHandle::invalid();
    }

    auto& slot = m_soundPool[slotIndex];
    slot.sound.setBuffer(it->second);
    slot.baseVolume = std::clamp(volume, 0.0f, 1.0f);                             // Store base volume
    slot.sound.setVolume(calculateEffectiveSFXVolume(slot.baseVolume) * 100.0f);  // SFML uses 0-100
    slot.sound.setPitch(pitch);
    slot.sound.setLoop(loop);
    slot.sound.setRelativeToListener(true);  // Non-spatial by default
    slot.sound.setPosition(0.0f, 0.0f, 0.0f);
    slot.sound.play();
    slot.inUse = true;

    AudioHandle handle;
    handle.index      = static_cast<uint32_t>(slotIndex);
    handle.generation = slot.generation;

    spdlog::debug("Playing SFX '{}' in slot {} (gen {})", id, slotIndex, slot.generation);
    return handle;
}

AudioHandle
SAudio::playSpatialSFX(const std::string& id, const Vec2& position, float volume, float pitch, bool loop, float minDistance, float attenuation)
{
    if (!m_initialized)
    {
        spdlog::error("Cannot play spatial SFX: audio system not initialized");
        return AudioHandle::invalid();
    }

    // Find the sound buffer
    auto it = m_soundBuffers.find(id);
    if (it == m_soundBuffers.end())
    {
        spdlog::error("Sound buffer '{}' not found", id);
        return AudioHandle::invalid();
    }

    // Find available slot
    int slotIndex = findAvailableSlot();
    if (slotIndex < 0)
    {
        spdlog::warn("Sound pool full, cannot play spatial '{}'", id);
        return AudioHandle::invalid();
    }

    auto& slot = m_soundPool[slotIndex];
    slot.sound.setBuffer(it->second);
    slot.baseVolume = std::clamp(volume, 0.0f, 1.0f);                             // Store base volume
    slot.sound.setVolume(calculateEffectiveSFXVolume(slot.baseVolume) * 100.0f);  // SFML uses 0-100
    slot.sound.setPitch(pitch);
    slot.sound.setLoop(loop);
    slot.sound.setRelativeToListener(false);  // Spatial audio
    slot.sound.setPosition(position.x, position.y, 0.0f);
    slot.sound.setMinDistance(minDistance);
    slot.sound.setAttenuation(attenuation);
    slot.sound.play();
    slot.inUse = true;

    AudioHandle handle;
    handle.index      = static_cast<uint32_t>(slotIndex);
    handle.generation = slot.generation;

    spdlog::debug("Playing spatial SFX '{}' at ({}, {}) in slot {} (gen {})", id, position.x, position.y, slotIndex, slot.generation);
    return handle;
}

void SAudio::stopSFX(AudioHandle handle)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    auto& slot = m_soundPool[handle.index];
    slot.sound.stop();
    slot.inUse = false;
    slot.generation++;
}

void SAudio::pauseSFX(AudioHandle handle)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    m_soundPool[handle.index].sound.pause();
}

void SAudio::resumeSFX(AudioHandle handle)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    auto& slot = m_soundPool[handle.index];
    if (slot.sound.getStatus() == sf::Sound::Paused)
    {
        slot.sound.play();
    }
}

bool SAudio::isPlayingSFX(AudioHandle handle) const
{
    if (!isHandleValid(handle))
    {
        return false;
    }

    return m_soundPool[handle.index].sound.getStatus() == sf::Sound::Playing;
}

void SAudio::setSFXPosition(AudioHandle handle, const Vec2& position)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    m_soundPool[handle.index].sound.setPosition(position.x, position.y, 0.0f);
}

void SAudio::setSFXVolume(AudioHandle handle, float volume)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    // Clamp and store base volume, then apply with master and category multipliers
    volume                               = std::clamp(volume, 0.0f, 1.0f);
    m_soundPool[handle.index].baseVolume = volume;
    m_soundPool[handle.index].sound.setVolume(calculateEffectiveSFXVolume(volume) * 100.0f);
}

bool SAudio::playMusic(const std::string& id, bool loop, float volume)
{
    if (!m_initialized)
    {
        spdlog::error("Cannot play music: audio system not initialized");
        return false;
    }

    // Find the music path
    auto it = m_musicPaths.find(id);
    if (it == m_musicPaths.end())
    {
        spdlog::error("Music '{}' not found", id);
        return false;
    }

    // Stop current music if playing
    if (m_currentMusic)
    {
        m_currentMusic->stop();
    }

    // Suppress ALSA errors during music loading in headless environment
#ifndef _WIN32
    int oldStderr = -1;
    int devNull   = open("/dev/null", O_WRONLY);
    if (devNull != -1)
    {
        oldStderr = dup(STDERR_FILENO);
        dup2(devNull, STDERR_FILENO);
        close(devNull);
    }
#endif

    // Create new music object
    m_currentMusic = std::make_unique<sf::Music>();
    bool success   = m_currentMusic->openFromFile(it->second);

    if (success)
    {
        m_currentMusicBaseVolume = std::clamp(volume, 0.0f, 1.0f);  // Store base volume
        m_currentMusic->setLoop(loop);
        m_currentMusic->setVolume(calculateEffectiveMusicVolume(m_currentMusicBaseVolume) * 100.0f);  // SFML uses 0-100
        m_currentMusic->play();
        m_currentMusicId = id;
    }

    // Restore stderr
#ifndef _WIN32
    if (oldStderr != -1)
    {
        dup2(oldStderr, STDERR_FILENO);
        close(oldStderr);
    }
#endif

    if (!success)
    {
        spdlog::error("Failed to open music file: {}", it->second);
        m_currentMusic.reset();
        return false;
    }

    spdlog::info("Playing music '{}' from '{}'", id, it->second);
    return true;
}

void SAudio::stopMusic()
{
    if (m_currentMusic)
    {
        m_currentMusic->stop();
        m_currentMusic.reset();
        m_currentMusicId.clear();
        spdlog::debug("Music stopped");
    }
}

void SAudio::pauseMusic()
{
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing)
    {
        m_currentMusic->pause();
        spdlog::debug("Music paused");
    }
}

void SAudio::resumeMusic()
{
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Paused)
    {
        m_currentMusic->play();
        spdlog::debug("Music resumed");
    }
}

bool SAudio::isMusicPlaying() const
{
    return m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing;
}

void SAudio::setMasterVolume(float volume)
{
    m_masterVolume = std::clamp(volume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);

    // Update all active sounds using their stored base volumes
    for (auto& slot : m_soundPool)
    {
        if (slot.inUse)
        {
            slot.sound.setVolume(calculateEffectiveSFXVolume(slot.baseVolume) * 100.0f);
        }
    }

    // Update music using its stored base volume
    if (m_currentMusic)
    {
        m_currentMusic->setVolume(calculateEffectiveMusicVolume(m_currentMusicBaseVolume) * 100.0f);
    }

    spdlog::debug("Master volume set to {}", m_masterVolume);
}

void SAudio::setSFXVolume(float volume)
{
    m_sfxVolume = std::clamp(volume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);

    // Update all active sounds using their stored base volumes
    for (auto& slot : m_soundPool)
    {
        if (slot.inUse)
        {
            slot.sound.setVolume(calculateEffectiveSFXVolume(slot.baseVolume) * 100.0f);
        }
    }

    spdlog::debug("SFX volume set to {}", m_sfxVolume);
}

void SAudio::setMusicVolume(float volume)
{
    m_musicVolume = std::clamp(volume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);

    // Update music using stored base volume
    if (m_currentMusic)
    {
        m_currentMusic->setVolume(calculateEffectiveMusicVolume(m_currentMusicBaseVolume) * 100.0f);
    }

    spdlog::debug("Music volume set to {}", m_musicVolume);
}

float SAudio::getMasterVolume() const
{
    return m_masterVolume;
}

float SAudio::getSFXVolume() const
{
    return m_sfxVolume;
}

float SAudio::getMusicVolume() const
{
    return m_musicVolume;
}

void SAudio::setListenerPosition(const Vec2& position)
{
    sf::Listener::setPosition(position.x, position.y, 0.0f);
}

void SAudio::update(float deltaTime)
{
    if (!m_initialized)
    {
        return;
    }

    // Update fade states for all active sounds
    for (auto& slot : m_soundPool)
    {
        if (slot.inUse)
        {
            // Update fade if active
            if (slot.fadeState != FadeState::None)
            {
                updateSoundFade(slot, deltaTime);
            }

            // Check if sound has finished playing
            sf::Sound::Status status = slot.sound.getStatus();
            if (status == sf::Sound::Stopped)
            {
                slot.inUse = false;
                slot.generation++;
                slot.fadeState = FadeState::None;
            }
        }
    }

    // Update music fade if active
    if (m_currentMusic && m_musicFadeState != FadeState::None)
    {
        updateMusicFade(deltaTime);
    }
}

void SAudio::update(float deltaTime, World& world)
{
    // First perform regular audio bookkeeping (fade states, freeing slots)
    update(deltaTime);

    if (!m_initialized)
    {
        return;
    }

    auto handleSourceCommand = [this](Components::CAudioSource& source, const Vec2& position, bool hasPosition) {
        const Vec2 finalPos = hasPosition ? position : Vec2{0.0f, 0.0f};

        switch (source.m_pendingCommand)
        {
            case Components::AudioCommand::Play:
                if (source.m_clipId.empty())
                {
                    spdlog::warn("CAudioSource: Cannot play, no clip ID set");
                    break;
                }

                if (source.m_type == AudioType::SFX)
                {
                    if (source.m_playHandle.isValid())
                    {
                        stopSFX(source.m_playHandle);
                    }

                    source.m_playHandle = source.m_spatial && hasPosition ? playSpatialSFX(source.m_clipId,
                                                                                         finalPos,
                                                                                         source.m_volume,
                                                                                         source.m_pitch,
                                                                                         source.m_loop,
                                                                                         source.m_minDistance,
                                                                                         source.m_attenuation)
                                                                              : playSFX(source.m_clipId,
                                                                                        source.m_volume,
                                                                                        source.m_pitch,
                                                                                        source.m_loop);
                    source.m_isPlaying = source.m_playHandle.isValid();
                }
                else
                {
                    source.m_isPlaying = playMusic(source.m_clipId, source.m_loop, source.m_volume);
                    source.m_playHandle = AudioHandle::invalid();
                }
                break;

            case Components::AudioCommand::Pause:
                if (source.m_type == AudioType::SFX && source.m_playHandle.isValid())
                {
                    pauseSFX(source.m_playHandle);
                }
                else if (source.m_type == AudioType::Music)
                {
                    pauseMusic();
                }
                break;

            case Components::AudioCommand::Stop:
                if (source.m_type == AudioType::SFX && source.m_playHandle.isValid())
                {
                    stopSFX(source.m_playHandle);
                }
                else if (source.m_type == AudioType::Music)
                {
                    stopMusic();
                }
                source.m_playHandle = AudioHandle::invalid();
                source.m_isPlaying  = false;
                break;

            case Components::AudioCommand::None:
                break;
        }

        source.m_pendingCommand = Components::AudioCommand::None;

        // Keep spatial sounds attached to the entity
        if (source.m_type == AudioType::SFX && source.m_spatial && source.m_playHandle.isValid() && hasPosition)
        {
            setSFXPosition(source.m_playHandle, finalPos);
        }

        // Refresh cached playing state
        if (source.m_type == AudioType::SFX)
        {
            source.m_isPlaying = source.m_playHandle.isValid() && isPlayingSFX(source.m_playHandle);

            if (!source.m_isPlaying)
            {
                source.m_playHandle = AudioHandle::invalid();
            }
        }
        else
        {
            source.m_isPlaying = isMusicPlaying();
        }
    };

    auto handleListenerSource = [this](Components::CAudioListener::AudioSourceConfig& config, const Vec2& listenerPos, bool hasPosition) {
        const Vec2 finalPos = hasPosition ? listenerPos : Vec2{0.0f, 0.0f};

        switch (config.pendingCommand)
        {
            case Components::AudioCommand::Play:
                if (config.clipId.empty())
                {
                    spdlog::warn("CAudioListener: Cannot play source with empty clipId");
                    break;
                }

                if (config.type == AudioType::SFX)
                {
                    if (config.playHandle.isValid())
                    {
                        stopSFX(config.playHandle);
                    }

                    config.playHandle = config.spatial && hasPosition ? playSpatialSFX(config.clipId,
                                                                                         finalPos,
                                                                                         config.volume,
                                                                                         config.pitch,
                                                                                         config.loop,
                                                                                         config.minDistance,
                                                                                         config.attenuation)
                                                                             : playSFX(config.clipId,
                                                                                       config.volume,
                                                                                       config.pitch,
                                                                                       config.loop);
                    config.isPlaying = config.playHandle.isValid();
                }
                else
                {
                    config.isPlaying = playMusic(config.clipId, config.loop, config.volume);
                }
                break;

            case Components::AudioCommand::Pause:
                if (config.type == AudioType::SFX && config.playHandle.isValid())
                {
                    pauseSFX(config.playHandle);
                }
                else if (config.type == AudioType::Music)
                {
                    pauseMusic();
                }
                break;

            case Components::AudioCommand::Stop:
                if (config.type == AudioType::SFX && config.playHandle.isValid())
                {
                    stopSFX(config.playHandle);
                }
                else if (config.type == AudioType::Music)
                {
                    stopMusic();
                }
                config.playHandle = AudioHandle::invalid();
                config.isPlaying  = false;
                break;

            case Components::AudioCommand::None:
                break;
        }

        config.pendingCommand = Components::AudioCommand::None;

        if (config.type == AudioType::SFX)
        {
            if (config.spatial && config.playHandle.isValid() && hasPosition)
            {
                setSFXPosition(config.playHandle, finalPos);
            }

            config.isPlaying = config.playHandle.isValid() && isPlayingSFX(config.playHandle);

            if (!config.isPlaying)
            {
                config.playHandle = AudioHandle::invalid();
            }
        }
        else
        {
            config.isPlaying = isMusicPlaying();
        }
    };

    // Synchronize listener position and named sources
    world.each<Components::CAudioListener>([this, &world, &handleListenerSource](Entity entity, Components::CAudioListener& listener) {
        const auto* transform   = world.tryGet<Components::CTransform>(entity);
        const bool  hasPosition = transform != nullptr;
        Vec2        listenerPos = hasPosition ? transform->getPosition() : Vec2{0.0f, 0.0f};

        if (listener.isDefaultListener() && hasPosition)
        {
            setListenerPosition(listenerPos);
        }

        for (auto& [name, config] : listener.getAudioSources())
        {
            (void)name;
            handleListenerSource(config, listenerPos, hasPosition);
        }
    });

    // Synchronize standalone audio sources (optionally spatial)
    world.each<Components::CAudioSource>([this, &world, &handleSourceCommand](Entity entity, Components::CAudioSource& source) {
        const auto* transform   = world.tryGet<Components::CTransform>(entity);
        const bool  hasPosition = transform != nullptr;
        Vec2        worldPos    = hasPosition ? transform->getPosition() : Vec2{0.0f, 0.0f};

        handleSourceCommand(source, worldPos, hasPosition);
    });
}

int SAudio::findAvailableSlot()
{
    for (size_t i = 0; i < m_soundPool.size(); ++i)
    {
        if (!m_soundPool[i].inUse)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool SAudio::isHandleValid(AudioHandle handle) const
{
    if (!handle.isValid() || handle.index >= m_soundPool.size())
    {
        return false;
    }

    const auto& slot = m_soundPool[handle.index];
    return slot.inUse && slot.generation == handle.generation;
}

float SAudio::calculateEffectiveSFXVolume(float baseVolume) const
{
    return std::clamp(baseVolume * m_sfxVolume * m_masterVolume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);
}

float SAudio::calculateEffectiveMusicVolume(float baseVolume) const
{
    return std::clamp(baseVolume * m_musicVolume * m_masterVolume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);
}

AudioHandle SAudio::playSFXWithFade(const std::string& id, float volume, float pitch, bool loop, const FadeConfig& fadeConfig)
{
    // Start at 0 volume if fading in, otherwise use target volume
    float       startVolume = (fadeConfig.duration > 0.0f) ? 0.0f : volume;
    AudioHandle handle      = playSFX(id, startVolume, pitch, loop);

    if (!handle.isValid() || fadeConfig.duration <= 0.0f)
    {
        return handle;
    }

    // Set up fade-in
    auto& slot          = m_soundPool[handle.index];
    slot.fadeState      = FadeState::FadingIn;
    slot.startVolume    = startVolume;
    slot.targetVolume   = volume;
    slot.fadeDuration   = fadeConfig.duration;
    slot.fadeElapsed    = 0.0f;
    slot.fadeCurve      = fadeConfig.curve;
    slot.allowInterrupt = fadeConfig.allowInterrupt;
    slot.onFadeComplete = fadeConfig.onComplete;
    slot.stopAfterFade  = false;

    return handle;
}

AudioHandle SAudio::playSpatialSFXWithFade(
    const std::string& id, const Vec2& position, float volume, float pitch, bool loop, float minDistance, float attenuation, const FadeConfig& fadeConfig)
{
    // Start at 0 volume if fading in, otherwise use target volume
    float       startVolume = (fadeConfig.duration > 0.0f) ? 0.0f : volume;
    AudioHandle handle      = playSpatialSFX(id, position, startVolume, pitch, loop, minDistance, attenuation);

    if (!handle.isValid() || fadeConfig.duration <= 0.0f)
    {
        return handle;
    }

    // Set up fade-in
    auto& slot          = m_soundPool[handle.index];
    slot.fadeState      = FadeState::FadingIn;
    slot.startVolume    = startVolume;
    slot.targetVolume   = volume;
    slot.fadeDuration   = fadeConfig.duration;
    slot.fadeElapsed    = 0.0f;
    slot.fadeCurve      = fadeConfig.curve;
    slot.allowInterrupt = fadeConfig.allowInterrupt;
    slot.onFadeComplete = fadeConfig.onComplete;
    slot.stopAfterFade  = false;

    return handle;
}

bool SAudio::fadeSFX(AudioHandle handle, float targetVolume, const FadeConfig& fadeConfig)
{
    if (!isHandleValid(handle))
    {
        return false;
    }

    auto& slot = m_soundPool[handle.index];

    // Check if current fade can be interrupted
    if (slot.fadeState != FadeState::None && !slot.allowInterrupt)
    {
        spdlog::warn("Cannot interrupt non-interruptible fade on sound in slot {}", handle.index);
        return false;
    }

    targetVolume = std::clamp(targetVolume, 0.0f, 1.0f);

    // Instant fade
    if (fadeConfig.duration <= 0.0f)
    {
        slot.baseVolume = targetVolume;
        slot.sound.setVolume(calculateEffectiveSFXVolume(targetVolume) * 100.0f);
        slot.fadeState = FadeState::None;
        if (fadeConfig.onComplete)
        {
            fadeConfig.onComplete();
        }
        return true;
    }

    // Set up fade
    slot.fadeState      = (targetVolume > slot.baseVolume) ? FadeState::FadingIn : FadeState::FadingOut;
    slot.startVolume    = slot.baseVolume;
    slot.targetVolume   = targetVolume;
    slot.fadeDuration   = fadeConfig.duration;
    slot.fadeElapsed    = 0.0f;
    slot.fadeCurve      = fadeConfig.curve;
    slot.allowInterrupt = fadeConfig.allowInterrupt;
    slot.onFadeComplete = fadeConfig.onComplete;
    slot.stopAfterFade  = false;

    return true;
}

void SAudio::stopSFXWithFade(AudioHandle handle, const FadeConfig& fadeConfig)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    // Instant stop
    if (fadeConfig.duration <= 0.0f)
    {
        stopSFX(handle);
        if (fadeConfig.onComplete)
        {
            fadeConfig.onComplete();
        }
        return;
    }

    auto& slot = m_soundPool[handle.index];

    // Check if current fade can be interrupted
    if (slot.fadeState != FadeState::None && !slot.allowInterrupt)
    {
        spdlog::warn("Cannot interrupt non-interruptible fade on sound in slot {}", handle.index);
        return;
    }

    // Set up fade-out to stop
    slot.fadeState      = FadeState::FadingOut;
    slot.startVolume    = slot.baseVolume;
    slot.targetVolume   = 0.0f;
    slot.fadeDuration   = fadeConfig.duration;
    slot.fadeElapsed    = 0.0f;
    slot.fadeCurve      = fadeConfig.curve;
    slot.allowInterrupt = fadeConfig.allowInterrupt;
    slot.onFadeComplete = fadeConfig.onComplete;
    slot.stopAfterFade  = true;
}

void SAudio::cancelFade(AudioHandle handle)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    auto& slot          = m_soundPool[handle.index];
    slot.fadeState      = FadeState::None;
    slot.onFadeComplete = nullptr;
    slot.stopAfterFade  = false;
}

bool SAudio::playMusicWithFade(const std::string& id, bool loop, float volume, const FadeConfig& fadeConfig)
{
    // Start at 0 volume if fading in, otherwise use target volume
    float startVolume = (fadeConfig.duration > 0.0f) ? 0.0f : volume;
    bool  success     = playMusic(id, loop, startVolume);

    if (!success || fadeConfig.duration <= 0.0f)
    {
        return success;
    }

    // Set up fade-in
    m_musicFadeState      = FadeState::FadingIn;
    m_musicStartVolume    = startVolume;
    m_musicTargetVolume   = volume;
    m_musicFadeDuration   = fadeConfig.duration;
    m_musicFadeElapsed    = 0.0f;
    m_musicFadeCurve      = fadeConfig.curve;
    m_musicAllowInterrupt = fadeConfig.allowInterrupt;
    m_musicOnFadeComplete = fadeConfig.onComplete;
    m_musicStopAfterFade  = false;

    return true;
}

bool SAudio::fadeMusic(float targetVolume, const FadeConfig& fadeConfig)
{
    if (!m_currentMusic)
    {
        return false;
    }

    // Check if current fade can be interrupted
    if (m_musicFadeState != FadeState::None && !m_musicAllowInterrupt)
    {
        spdlog::warn("Cannot interrupt non-interruptible music fade");
        return false;
    }

    targetVolume = std::clamp(targetVolume, 0.0f, 1.0f);

    // Instant fade
    if (fadeConfig.duration <= 0.0f)
    {
        m_currentMusicBaseVolume = targetVolume;
        m_currentMusic->setVolume(calculateEffectiveMusicVolume(targetVolume) * 100.0f);
        m_musicFadeState = FadeState::None;
        if (fadeConfig.onComplete)
        {
            fadeConfig.onComplete();
        }
        return true;
    }

    // Set up fade
    m_musicFadeState      = (targetVolume > m_currentMusicBaseVolume) ? FadeState::FadingIn : FadeState::FadingOut;
    m_musicStartVolume    = m_currentMusicBaseVolume;
    m_musicTargetVolume   = targetVolume;
    m_musicFadeDuration   = fadeConfig.duration;
    m_musicFadeElapsed    = 0.0f;
    m_musicFadeCurve      = fadeConfig.curve;
    m_musicAllowInterrupt = fadeConfig.allowInterrupt;
    m_musicOnFadeComplete = fadeConfig.onComplete;
    m_musicStopAfterFade  = false;

    return true;
}

void SAudio::stopMusicWithFade(const FadeConfig& fadeConfig)
{
    if (!m_currentMusic)
    {
        return;
    }

    // Instant stop
    if (fadeConfig.duration <= 0.0f)
    {
        stopMusic();
        if (fadeConfig.onComplete)
        {
            fadeConfig.onComplete();
        }
        return;
    }

    // Check if current fade can be interrupted
    if (m_musicFadeState != FadeState::None && !m_musicAllowInterrupt)
    {
        spdlog::warn("Cannot interrupt non-interruptible music fade");
        return;
    }

    // Set up fade-out to stop
    m_musicFadeState      = FadeState::FadingOut;
    m_musicStartVolume    = m_currentMusicBaseVolume;
    m_musicTargetVolume   = 0.0f;
    m_musicFadeDuration   = fadeConfig.duration;
    m_musicFadeElapsed    = 0.0f;
    m_musicFadeCurve      = fadeConfig.curve;
    m_musicAllowInterrupt = fadeConfig.allowInterrupt;
    m_musicOnFadeComplete = fadeConfig.onComplete;
    m_musicStopAfterFade  = true;
}

void SAudio::cancelMusicFade()
{
    m_musicFadeState      = FadeState::None;
    m_musicOnFadeComplete = nullptr;
    m_musicStopAfterFade  = false;
}

float SAudio::applyFadeCurve(float t, FadeCurve curve) const
{
    t = std::clamp(t, 0.0f, 1.0f);

    switch (curve)
    {
        case FadeCurve::Linear:
            return t;

        case FadeCurve::EaseIn:
            // Quadratic ease-in: slow start, fast end
            return t * t;

        case FadeCurve::EaseOut:
            // Quadratic ease-out: fast start, slow end
            return t * (2.0f - t);

        case FadeCurve::EaseInOut:
            // Smoothstep: slow start and end, fast middle
            return t * t * (3.0f - 2.0f * t);

        default:
            return t;
    }
}

void SAudio::updateSoundFade(SoundSlot& slot, float deltaTime)
{
    slot.fadeElapsed += deltaTime;

    // Calculate normalized time (0.0 to 1.0)
    float t = (slot.fadeDuration > 0.0f) ? (slot.fadeElapsed / slot.fadeDuration) : 1.0f;

    if (t >= 1.0f)
    {
        // Fade complete
        slot.baseVolume = slot.targetVolume;
        slot.sound.setVolume(calculateEffectiveSFXVolume(slot.targetVolume) * 100.0f);
        slot.fadeState = FadeState::None;

        // Invoke callback
        if (slot.onFadeComplete)
        {
            auto callback       = slot.onFadeComplete;
            slot.onFadeComplete = nullptr;
            callback();
        }

        // Stop if requested
        if (slot.stopAfterFade)
        {
            slot.sound.stop();
            slot.inUse = false;
            slot.generation++;
        }
    }
    else
    {
        // Interpolate volume with curve
        float curvedT   = applyFadeCurve(t, slot.fadeCurve);
        float newVolume = slot.startVolume + (slot.targetVolume - slot.startVolume) * curvedT;
        slot.baseVolume = newVolume;
        slot.sound.setVolume(calculateEffectiveSFXVolume(newVolume) * 100.0f);
    }
}

void SAudio::updateMusicFade(float deltaTime)
{
    m_musicFadeElapsed += deltaTime;

    // Calculate normalized time (0.0 to 1.0)
    float t = (m_musicFadeDuration > 0.0f) ? (m_musicFadeElapsed / m_musicFadeDuration) : 1.0f;

    if (t >= 1.0f)
    {
        // Fade complete
        m_currentMusicBaseVolume = m_musicTargetVolume;
        if (m_currentMusic)
        {
            m_currentMusic->setVolume(calculateEffectiveMusicVolume(m_musicTargetVolume) * 100.0f);
        }
        m_musicFadeState = FadeState::None;

        // Invoke callback
        if (m_musicOnFadeComplete)
        {
            auto callback         = m_musicOnFadeComplete;
            m_musicOnFadeComplete = nullptr;
            callback();
        }

        // Stop if requested
        if (m_musicStopAfterFade)
        {
            stopMusic();
        }
    }
    else
    {
        // Interpolate volume with curve
        float curvedT            = applyFadeCurve(t, m_musicFadeCurve);
        float newVolume          = m_musicStartVolume + (m_musicTargetVolume - m_musicStartVolume) * curvedT;
        m_currentMusicBaseVolume = newVolume;
        if (m_currentMusic)
        {
            m_currentMusic->setVolume(calculateEffectiveMusicVolume(newVolume) * 100.0f);
        }
    }
}

}  // namespace Systems
