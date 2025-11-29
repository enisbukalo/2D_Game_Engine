#include "SAudioSystem.h"
#include <SFML/Audio.hpp>
#include <algorithm>
#include <spdlog/spdlog.h>

#ifndef _WIN32
    #include <unistd.h>
    #include <fcntl.h>
#endif

SAudioSystem::SAudioSystem(size_t poolSize) : m_soundPool(poolSize) {}

SAudioSystem::~SAudioSystem()
{
    shutdown();
}

SAudioSystem& SAudioSystem::instance()
{
    static SAudioSystem instance;
    return instance;
}

bool SAudioSystem::initialize()
{
    if (m_initialized)
    {
        spdlog::warn("SAudioSystem already initialized");
        return true;
    }

    spdlog::info("Initializing SFML audio system with pool size: {}", m_soundPool.size());

    // Suppress ALSA/OpenAL errors in headless environments
#ifndef _WIN32
    int oldStderr = -1;
    int devNull = open("/dev/null", O_WRONLY);
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

void SAudioSystem::shutdown()
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

bool SAudioSystem::loadSound(const std::string& id, const std::string& filepath, AudioType type)
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
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull != -1)
        {
            oldStderr = dup(STDERR_FILENO);
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }
#endif

        sf::SoundBuffer buffer;
        bool success = buffer.loadFromFile(filepath);

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

void SAudioSystem::unloadSound(const std::string& id)
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

AudioHandle SAudioSystem::playSFX(const std::string& id, float volume, float pitch, bool loop)
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
    slot.sound.setVolume(calculateEffectiveSFXVolume(volume) * 100.0f);  // SFML uses 0-100
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

AudioHandle SAudioSystem::playSpatialSFX(const std::string& id,
                                         const Vec2&        position,
                                         float              volume,
                                         float              pitch,
                                         bool               loop,
                                         float              minDistance,
                                         float              attenuation)
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
    slot.sound.setVolume(calculateEffectiveSFXVolume(volume) * 100.0f);  // SFML uses 0-100
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

void SAudioSystem::stopSFX(AudioHandle handle)
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

void SAudioSystem::pauseSFX(AudioHandle handle)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    m_soundPool[handle.index].sound.pause();
}

void SAudioSystem::resumeSFX(AudioHandle handle)
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

bool SAudioSystem::isPlayingSFX(AudioHandle handle) const
{
    if (!isHandleValid(handle))
    {
        return false;
    }

    return m_soundPool[handle.index].sound.getStatus() == sf::Sound::Playing;
}

void SAudioSystem::setSFXPosition(AudioHandle handle, const Vec2& position)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    m_soundPool[handle.index].sound.setPosition(position.x, position.y, 0.0f);
}

void SAudioSystem::setSFXVolume(AudioHandle handle, float volume)
{
    if (!isHandleValid(handle))
    {
        return;
    }

    // Clamp and apply volume with master and category multipliers
    volume = std::clamp(volume, 0.0f, 1.0f);
    m_soundPool[handle.index].sound.setVolume(calculateEffectiveSFXVolume(volume) * 100.0f);
}

bool SAudioSystem::playMusic(const std::string& id, bool loop, float volume)
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
    int devNull = open("/dev/null", O_WRONLY);
    if (devNull != -1)
    {
        oldStderr = dup(STDERR_FILENO);
        dup2(devNull, STDERR_FILENO);
        close(devNull);
    }
#endif

    // Create new music object
    m_currentMusic = std::make_unique<sf::Music>();
    bool success = m_currentMusic->openFromFile(it->second);

    if (success)
    {
        m_currentMusic->setLoop(loop);
        m_currentMusic->setVolume(calculateEffectiveMusicVolume(volume) * 100.0f);  // SFML uses 0-100
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

void SAudioSystem::stopMusic()
{
    if (m_currentMusic)
    {
        m_currentMusic->stop();
        m_currentMusic.reset();
        m_currentMusicId.clear();
        spdlog::debug("Music stopped");
    }
}

void SAudioSystem::pauseMusic()
{
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing)
    {
        m_currentMusic->pause();
        spdlog::debug("Music paused");
    }
}

void SAudioSystem::resumeMusic()
{
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Paused)
    {
        m_currentMusic->play();
        spdlog::debug("Music resumed");
    }
}

bool SAudioSystem::isMusicPlaying() const
{
    return m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing;
}

void SAudioSystem::setMasterVolume(float volume)
{
    m_masterVolume = std::clamp(volume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);

    // Update all active sounds
    for (auto& slot : m_soundPool)
    {
        if (slot.inUse)
        {
            float currentVolume = slot.sound.getVolume() / 100.0f;
            slot.sound.setVolume(calculateEffectiveSFXVolume(currentVolume) * 100.0f);
        }
    }

    // Update music
    if (m_currentMusic)
    {
        float currentVolume = m_currentMusic->getVolume() / 100.0f;
        m_currentMusic->setVolume(calculateEffectiveMusicVolume(currentVolume) * 100.0f);
    }

    spdlog::debug("Master volume set to {}", m_masterVolume);
}

void SAudioSystem::setSFXVolume(float volume)
{
    m_sfxVolume = std::clamp(volume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);

    // Update all active sounds
    for (auto& slot : m_soundPool)
    {
        if (slot.inUse)
        {
            float currentVolume = slot.sound.getVolume() / 100.0f;
            slot.sound.setVolume(calculateEffectiveSFXVolume(currentVolume) * 100.0f);
        }
    }

    spdlog::debug("SFX volume set to {}", m_sfxVolume);
}

void SAudioSystem::setMusicVolume(float volume)
{
    m_musicVolume = std::clamp(volume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);

    // Update music
    if (m_currentMusic)
    {
        float currentVolume = m_currentMusic->getVolume() / 100.0f;
        m_currentMusic->setVolume(calculateEffectiveMusicVolume(currentVolume) * 100.0f);
    }

    spdlog::debug("Music volume set to {}", m_musicVolume);
}

float SAudioSystem::getMasterVolume() const
{
    return m_masterVolume;
}

float SAudioSystem::getSFXVolume() const
{
    return m_sfxVolume;
}

float SAudioSystem::getMusicVolume() const
{
    return m_musicVolume;
}

void SAudioSystem::setListenerPosition(const Vec2& position)
{
    sf::Listener::setPosition(position.x, position.y, 0.0f);
}

void SAudioSystem::update(float deltaTime)
{
    if (!m_initialized)
    {
        return;
    }

    // Free finished sounds from the pool
    for (auto& slot : m_soundPool)
    {
        if (slot.inUse)
        {
            sf::Sound::Status status = slot.sound.getStatus();
            if (status == sf::Sound::Stopped)
            {
                slot.inUse = false;
                slot.generation++;
            }
        }
    }
}

int SAudioSystem::findAvailableSlot()
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

bool SAudioSystem::isHandleValid(AudioHandle handle) const
{
    if (!handle.isValid() || handle.index >= m_soundPool.size())
    {
        return false;
    }

    const auto& slot = m_soundPool[handle.index];
    return slot.inUse && slot.generation == handle.generation;
}

float SAudioSystem::calculateEffectiveSFXVolume(float baseVolume) const
{
    return std::clamp(baseVolume * m_sfxVolume * m_masterVolume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);
}

float SAudioSystem::calculateEffectiveMusicVolume(float baseVolume) const
{
    return std::clamp(baseVolume * m_musicVolume * m_masterVolume, AudioConstants::MIN_VOLUME, AudioConstants::MAX_VOLUME);
}
