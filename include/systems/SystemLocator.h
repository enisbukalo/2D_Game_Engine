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
    static void provideInput(SInput* input)
    {
        s_input = input;
    }
    static void providePhysics(S2DPhysics* physics)
    {
        s_physics = physics;
    }
    static void provideRenderer(SRenderer* renderer)
    {
        s_renderer = renderer;
    }
    static void provideParticle(SParticle* particle)
    {
        s_particle = particle;
    }
    static void provideAudio(SAudio* audio)
    {
        s_audio = audio;
    }

    static SInput& input()
    {
        assert(s_input && "Input system not set");
        return *s_input;
    }
    static S2DPhysics& physics()
    {
        assert(s_physics && "Physics system not set");
        return *s_physics;
    }
    static SRenderer& renderer()
    {
        assert(s_renderer && "Renderer system not set");
        return *s_renderer;
    }
    static SParticle& particle()
    {
        assert(s_particle && "Particle system not set");
        return *s_particle;
    }
    static SAudio& audio()
    {
        assert(s_audio && "Audio system not set");
        return *s_audio;
    }

    static SInput* tryInput()
    {
        return s_input;
    }
    static S2DPhysics* tryPhysics()
    {
        return s_physics;
    }
    static SRenderer* tryRenderer()
    {
        return s_renderer;
    }
    static SParticle* tryParticle()
    {
        return s_particle;
    }
    static SAudio* tryAudio()
    {
        return s_audio;
    }

private:
    inline static SInput*     s_input    = nullptr;
    inline static S2DPhysics* s_physics  = nullptr;
    inline static SRenderer*  s_renderer = nullptr;
    inline static SParticle*  s_particle = nullptr;
    inline static SAudio*     s_audio    = nullptr;
};

}  // namespace Systems
