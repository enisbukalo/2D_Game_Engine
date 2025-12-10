#pragma once

#include <Components.h>
#include <GameEngine.h>
#include "AudioTypes.h"

#include <iostream>
#include <memory>

/**
 * AudioManager Entity
 * Handles audio system initialization and volume control inputs
 */
class AudioManager : public Entity::Entity
{
private:
    GameEngine*                   m_gameEngine;
    Components::CInputController* m_input = nullptr;

    static constexpr float VOLUME_ADJUSTMENT_STEP = 0.05f;  // 5% per key press
    static constexpr float MAX_MUSIC_VOLUME       = 0.80f;  // 80% max volume
    static constexpr float INITIAL_VOLUME         = 0.15f;  // 15% initial volume

    friend class Systems::SEntity;

    AudioManager(const std::string& tag, size_t id, GameEngine* gameEngine) : Entity(tag, id), m_gameEngine(gameEngine)
    {
    }

public:
    void init() override
    {
        if (!m_gameEngine)
        {
            std::cerr << "AudioManager: Invalid game engine" << std::endl;
            return;
        }

        auto& audioSystem = m_gameEngine->getAudioSystem();

        // Set initial master volume
        audioSystem.setMasterVolume(INITIAL_VOLUME);
        std::cout << "AudioManager: Master volume set to " << static_cast<int>(audioSystem.getMasterVolume() * 100.0f)
                  << "%" << std::endl;

        // Load audio assets
        audioSystem.loadSound("background_music", "assets/audio/rainyday.mp3", AudioType::Music);
        audioSystem.loadSound("motor_boat", "assets/audio/motor_boat.mp3", AudioType::SFX);

        // Start background music
        audioSystem.playMusic("background_music", true, MAX_MUSIC_VOLUME);

        // Add input controller component and bind volume controls
        m_input = addComponent<Components::CInputController>();
        bindVolumeControls();

        std::cout << "AudioManager: Audio system initialized" << std::endl;
    }

    void update(float deltaTime) override
    {
        // Audio system updates are handled by GameEngine
    }

private:
    void bindVolumeControls()
    {
        if (!m_input || !m_gameEngine)
            return;

        auto& inputManager = m_gameEngine->getInputManager();

        // Register volume control actions
        ActionBinding volumeUp;
        volumeUp.keys.push_back(KeyCode::Up);
        volumeUp.trigger = ActionTrigger::Pressed;
        inputManager.bindAction("VolumeUp", volumeUp);

        ActionBinding volumeDown;
        volumeDown.keys.push_back(KeyCode::Down);
        volumeDown.trigger = ActionTrigger::Pressed;
        inputManager.bindAction("VolumeDown", volumeDown);

        // Set callbacks for volume control
        m_input->setActionCallback("VolumeUp",
                                   [this](ActionState state)
                                   {
                                       if (state == ActionState::Pressed)
                                       {
                                           auto& audioSystem   = m_gameEngine->getAudioSystem();
                                           float currentVolume = audioSystem.getMasterVolume();
                                           float newVolume     = std::min(currentVolume + VOLUME_ADJUSTMENT_STEP, 1.0f);
                                           audioSystem.setMasterVolume(newVolume);
                                           std::cout << "Master Volume: " << static_cast<int>(newVolume * 100.0f) << "%"
                                                     << std::endl;
                                       }
                                   });

        m_input->setActionCallback("VolumeDown",
                                   [this](ActionState state)
                                   {
                                       if (state == ActionState::Pressed)
                                       {
                                           auto& audioSystem   = m_gameEngine->getAudioSystem();
                                           float currentVolume = audioSystem.getMasterVolume();
                                           float newVolume     = std::max(currentVolume - VOLUME_ADJUSTMENT_STEP, 0.0f);
                                           audioSystem.setMasterVolume(newVolume);
                                           std::cout << "Master Volume: " << static_cast<int>(newVolume * 100.0f) << "%"
                                                     << std::endl;
                                       }
                                   });
    }
};
