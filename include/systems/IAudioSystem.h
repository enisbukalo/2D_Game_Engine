#ifndef IAUDIOSYSTEM_H
#define IAUDIOSYSTEM_H

#include <string>
#include "AudioTypes.h"
#include "Entity.h"

class IAudioSystem
{
public:
    virtual ~IAudioSystem() = default;

    virtual bool initialize() = 0;
    virtual void shutdown()   = 0;

    virtual bool loadSound(const std::string& id, const std::string& filepath, AudioType type) = 0;
    virtual void unloadSound(const std::string& id)                                            = 0;

    virtual bool playSfx(Entity entity, const std::string& id, bool loop = false, float volume = 1.0f) = 0;
    virtual void stopSfx(Entity entity)                                                                = 0;
    virtual void setSfxVolume(Entity entity, float volume)                                             = 0;

    virtual bool playMusic(const std::string& id, bool loop = true) = 0;
    virtual void stopMusic()                                        = 0;
    virtual void pauseMusic()                                       = 0;
    virtual void resumeMusic()                                      = 0;
    virtual bool isMusicPlaying() const                             = 0;

    virtual void  setMasterVolume(float volume) = 0;
    virtual void  setMusicVolume(float volume)  = 0;
    virtual float getMasterVolume() const       = 0;
    virtual float getMusicVolume() const        = 0;

    virtual void update(float deltaTime) = 0;
};

#endif  // IAUDIOSYSTEM_H
