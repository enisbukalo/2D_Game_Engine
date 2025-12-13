#include "AudioManager.h"

#include "InputHelpers.h"

#include <algorithm>
#include <iostream>

#include <ActionBinding.h>
#include <AudioTypes.h>
#include <Components.h>
#include <SAudio.h>
#include <SystemLocator.h>
#include <World.h>

namespace Example
{

void AudioManagerScript::onCreate(Entity self, World& world)
{
    auto& audio = Systems::SystemLocator::audio();

    // Set initial master volume (matches the old Example defaults)
    audio.setMasterVolume(kInitialMasterVolume);
    m_currentMasterVolume = kInitialMasterVolume;
    std::cout << "AudioManager: Master volume set to " << static_cast<int>(m_currentMasterVolume * 100.0f) << "%" << std::endl;

    // Load audio assets
    audio.loadSound("background_music", "assets/audio/rainyday.mp3", AudioType::Music);
    audio.loadSound("motor_boat", "assets/audio/motor_boat.mp3", AudioType::SFX);
    audio.loadSound("sway", "assets/audio/sway.mp3", AudioType::SFX);

    // Start background music
    audio.playMusic("background_music", true);
    audio.setMusicVolume(kMusicVolume);

    // Bind input actions for volume control
    auto* input = world.components().tryGet<Components::CInputController>(self);
    if (!input)
    {
        input = world.components().add<Components::CInputController>(self);
    }

    {
        ActionBinding up;
        up.keys        = {KeyCode::Up};
        up.trigger     = ActionTrigger::Pressed;
        up.allowRepeat = true;
        input->bindings["VolumeUp"].push_back({up, 0});
    }
    {
        ActionBinding down;
        down.keys        = {KeyCode::Down};
        down.trigger     = ActionTrigger::Pressed;
        down.allowRepeat = true;
        input->bindings["VolumeDown"].push_back({down, 0});
    }

    std::cout << "Audio initialized. Use Up/Down arrows to adjust volume." << std::endl;
}

void AudioManagerScript::onUpdate(float /*deltaTime*/, Entity self, World& world)
{
    auto* input = world.components().tryGet<Components::CInputController>(self);
    if (!input)
    {
        return;
    }

    if (isActionActive(*input, "VolumeUp"))
    {
        adjustMasterVolume(+kVolumeStep);
    }
    if (isActionActive(*input, "VolumeDown"))
    {
        adjustMasterVolume(-kVolumeStep);
    }
}

void AudioManagerScript::adjustMasterVolume(float delta)
{
    m_currentMasterVolume = std::clamp(m_currentMasterVolume + delta, 0.0f, 1.0f);
    Systems::SystemLocator::audio().setMasterVolume(m_currentMasterVolume);
    std::cout << "Master volume: " << static_cast<int>(m_currentMasterVolume * 100.0f) << "%" << std::endl;
}

}  // namespace Example
