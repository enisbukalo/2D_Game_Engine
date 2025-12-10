#include <GameEngine.h>
#include <Vec2.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <sstream>

#include "AudioManager.h"
#include "BarrelSpawner.h"
#include "BoatEntity.h"

using namespace Components;
using namespace Entity;
using namespace Systems;

// Define Screen Size
const int   SCREEN_WIDTH     = 1600;
const int   SCREEN_HEIGHT    = 1000;
const Vec2  GRAVITY          = {0.0f, 0.0f};  // Box2D gravity (m/s²), negative = downward
const float PIXELS_PER_METER = 100.0f;        // Rendering scale: 100 pixels = 1 meter

// Derived playfield size in meters (screen dimensions divided by scale)
const float PLAYFIELD_WIDTH_METERS  = SCREEN_WIDTH / PIXELS_PER_METER;   // 16 m
const float PLAYFIELD_HEIGHT_METERS = SCREEN_HEIGHT / PIXELS_PER_METER;  // 10 m

const size_t DEFAULT_BARREL_COUNT = 20;

class FishingGame
{
private:
    std::unique_ptr<GameEngine>     m_gameEngine;
    sf::Font                        m_font;
    bool                            m_running;
    bool                            m_fontLoaded;
    std::shared_ptr<Entity::Entity> m_oceanBackground;

    std::shared_ptr<Boat>         m_boat;
    std::shared_ptr<AudioManager> m_audioManager;

public:
    FishingGame() : m_running(true), m_fontLoaded(false), m_oceanBackground(nullptr)
    {
        // Create window configuration
        WindowConfig windowConfig;
        windowConfig.width      = SCREEN_WIDTH;
        windowConfig.height     = SCREEN_HEIGHT;
        windowConfig.title      = "Boat Example - ECS Framework";
        windowConfig.vsync      = true;
        windowConfig.frameLimit = 144;

        // Initialize game engine with window config and particle scale
        m_gameEngine = std::make_unique<GameEngine>(windowConfig, GRAVITY);

        // Try to load a system font (optional, will work without it)
        if (!m_font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            std::cout << "Could not load font. UI text will not be displayed." << std::endl;
        }
        else
        {
            m_fontLoaded = true;
        }
    }

    ~FishingGame()
    {
        // Cleanup audio
        m_gameEngine->getAudioSystem().shutdown();
    }

    void init()
    {
        // Create AudioManager entity to handle audio initialization and volume controls
        m_audioManager = m_gameEngine->spawn<AudioManager>("audio_manager", m_gameEngine.get());

        // Input Manager is already initialized by GameEngine - just disable ImGui passthrough
        m_gameEngine->getInputManager().setPassToImGui(false);

        // Set up Box2D physics world (gravity disabled)
        auto& physics = m_gameEngine->getPhysics();
        physics.setGravity({0.0f, 0.0f});

        // Build world procedurally
        m_boat = m_gameEngine->spawn<Boat>("player", &m_gameEngine->getInputManager(), &m_gameEngine->getAudioSystem());
        m_gameEngine->spawn<BarrelSpawner>("barrel_spawner", m_gameEngine.get(), 0, PLAYFIELD_WIDTH_METERS, 0, PLAYFIELD_HEIGHT_METERS, DEFAULT_BARREL_COUNT);

        std::cout << "Game initialized!" << std::endl;
        std::cout << "Physics: Box2D v3.1.1 (1 unit = 1 meter, Y-up)" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD            : Move player boat (W=forward, S=backward, A/D=turn when moving forward)" << std::endl;
    }

    void update(float dt)
    {
        // GameEngine handles all system updates (input, physics, particles, audio, entities)
        m_gameEngine->update(dt);
    }

    void render()
    {
        auto* window = m_gameEngine->getRenderer().getWindow();
        if (!window)
            return;

        // Use GameEngine's complete render pipeline (includes particles)
        m_gameEngine->render();

        // Draw UI text showing current status
        if (m_fontLoaded)
        {
            const auto& audioSystem   = m_gameEngine->getAudioSystem();
            float       currentVolume = audioSystem.getMasterVolume();

            std::ostringstream oss;
            oss << "Boat Example - ECS Rendering\n";
            oss << "Volume: " << static_cast<int>(currentVolume * 100.0f) << "% (Use Up/Down to adjust)";

            sf::Text text;
            text.setFont(m_font);
            text.setString(oss.str());
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::White);
            text.setPosition(10.0f, 10.0f);
            window->draw(text);
        }
    }

    void run()
    {
        init();

        sf::Clock clock;
        clock.restart();  // Reset clock after init to get proper first frame delta
        auto* window = m_gameEngine->getRenderer().getWindow();
        while (m_running && window && window->isOpen())
        {
            float dt = clock.restart().asSeconds();

            update(dt);
            render();
        }

        if (window)
        {
            window->close();
        }
    }
};

int main()
{
    try
    {
        FishingGame game;
        game.run();
    }
    catch (const std::exception& e)
    {
        std::cout << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
