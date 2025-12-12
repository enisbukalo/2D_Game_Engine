#ifndef CAUDIOSOURCE_H
#define CAUDIOSOURCE_H

#include <string>
#include "AudioTypes.h"

namespace Components
{

struct CAudioSource
{
    std::string clipId{};
    float       volume        = AudioConstants::DEFAULT_SFX_VOLUME;
    bool        loop          = false;
    bool        playRequested = false;
    bool        stopRequested = false;
};

}  // namespace Components

#endif  // CAUDIOSOURCE_H
