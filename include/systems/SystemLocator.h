#pragma once

#include <cassert>

namespace Systems
{
class SInput;
class S2DPhysics;
class SRenderer;
class SParticle;
class SAudio;

class SystemLocator
{
public:
    static void provideInput(SInput* input);
    static void providePhysics(S2DPhysics* physics);
    static void provideRenderer(SRenderer* renderer);
    static void provideParticle(SParticle* particle);
    static void provideAudio(SAudio* audio);

    static SInput&     input();
    static S2DPhysics& physics();
    static SRenderer&  renderer();
    static SParticle&  particle();
    static SAudio&     audio();

    static SInput*     tryInput();
    static S2DPhysics* tryPhysics();
    static SRenderer*  tryRenderer();
    static SParticle*  tryParticle();
    static SAudio*     tryAudio();
};

}  // namespace Systems
