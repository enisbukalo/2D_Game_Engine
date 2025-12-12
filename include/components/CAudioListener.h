#ifndef CAUDIOLISTENER_H
#define CAUDIOLISTENER_H

#include "AudioTypes.h"

namespace Components
{

struct CAudioListener
{
    float masterVolume = AudioConstants::DEFAULT_MASTER_VOLUME;
    float musicVolume  = AudioConstants::DEFAULT_MUSIC_VOLUME;
};

}  // namespace Components

#endif  // CAUDIOLISTENER_H
