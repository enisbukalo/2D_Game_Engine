#include <GameEngine.h>

#include <SFML/Graphics.hpp>

#include <Color.h>
#include <Components.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <limits>

#include "AudioManager.h"
#include "BarrelSpawner.h"
#include "BoatEntity.h"

using namespace Systems;

// Define Screen Size
static constexpr int   SCREEN_WIDTH  = 1600;
static constexpr int   SCREEN_HEIGHT = 1000;
static const Vec2      GRAVITY{0.0f, 0.0f};  // Y-up, meters/sec^2
static constexpr float PIXELS_PER_METER = 100.0f;

// Derived playfield size in meters (screen dimensions divided by scale)
static constexpr float PLAYFIELD_WIDTH_METERS  = SCREEN_WIDTH / PIXELS_PER_METER;
static constexpr float PLAYFIELD_HEIGHT_METERS = SCREEN_HEIGHT / PIXELS_PER_METER;

static constexpr size_t DEFAULT_BARREL_COUNT = 20;

static Entity createAudioManager(World& world)
{
    Entity audioManager = world.createEntity();
    auto*  script       = world.components().add<Components::CNativeScript>(audioManager);
    script->bind<Example::AudioManagerScript>();
    return audioManager;
}

static Entity createBarrelSpawner(World& world)
{
    Entity spawner = world.createEntity();
    auto*  script  = world.components().add<Components::CNativeScript>(spawner);
    script->bind<Example::BarrelSpawnerScript>(0.0f, PLAYFIELD_WIDTH_METERS, 0.0f, PLAYFIELD_HEIGHT_METERS, DEFAULT_BARREL_COUNT);
    return spawner;
}

int main()
{
    std::ofstream logFile("game_log.txt");

    try
    {
        logFile << "Starting game initialization...\n";
        logFile.flush();

        WindowConfig windowConfig;
        windowConfig.width      = SCREEN_WIDTH;
        windowConfig.height     = SCREEN_HEIGHT;
        windowConfig.title      = "Boat Example - ECS Framework";
        windowConfig.vsync      = true;
        windowConfig.frameLimit = 144;

        logFile << "Creating GameEngine...\n";
        logFile.flush();

        GameEngine engine(windowConfig, GRAVITY);

        logFile << "Configuring input manager...\n";
        logFile.flush();

        // Input Manager is already initialized by GameEngine - just disable ImGui passthrough
        engine.getInputManager().setPassToImGui(false);

        logFile << "Setting up physics...\n";
        logFile.flush();

        // Set up Box2D physics world (gravity disabled)
        engine.getPhysics().setGravity({0.0f, 0.0f});

        logFile << "Creating entities...\n";
        logFile.flush();

        World& world = engine.world();
        (void)createAudioManager(world);
        (void)Example::spawnBoat(world);
        (void)createBarrelSpawner(world);

        std::cout << "Game initialized!\n";
        std::cout << "Physics: Box2D v3.1.1 (1 unit = 1 meter, Y-up)\n";

        logFile << "Game initialized successfully!\n";
        logFile.flush();

        sf::Clock clock;
        clock.restart();

        auto* window = engine.getRenderer().getWindow();

        logFile << "Entering main loop...\n";
        logFile << "Window pointer: " << (window ? "valid" : "null") << "\n";
        if (window)
        {
            logFile << "Window is open: " << (window->isOpen() ? "yes" : "no") << "\n";
        }
        logFile << "Engine is running: " << (engine.is_running() ? "yes" : "no") << "\n";
        logFile.flush();

        int frameCount = 0;
        while (engine.is_running() && window && window->isOpen())
        {
            frameCount++;
            logFile << "Starting frame " << frameCount << "\n";
            logFile.flush();

            if (frameCount % 60 == 0)
            {
                logFile << "Reached frame " << frameCount << "\n";
                logFile.flush();
            }

            logFile << "  Getting delta time...\n";
            logFile.flush();
            const float dt     = clock.restart().asSeconds();
            const float safeDt = (dt < 0.001f) ? 0.016f : dt;  // Use 60 FPS default if dt is too small

            logFile << "  Calling engine.update(" << safeDt << ")...\n";
            logFile.flush();
            engine.update(safeDt);

            logFile << "  Calling engine.render()...\n";
            logFile.flush();
            engine.render();

            logFile << "  Frame " << frameCount << " complete\n";
            logFile.flush();
        }

        logFile << "Main loop exited after " << frameCount << " frames\n";
        logFile << "Engine is running: " << (engine.is_running() ? "yes" : "no") << "\n";
        if (window)
        {
            logFile << "Window is open: " << (window->isOpen() ? "yes" : "no") << "\n";
        }
        else
        {
            logFile << "Window is null\n";
        }
        logFile.flush();

        if (window)
        {
            window->close();
        }

        std::cout << "\nGame ended. Press Enter to exit...\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();

        logFile << "Exiting normally\n";
        logFile.close();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "FATAL std::exception: " << e.what() << "\n";
        logFile << "FATAL std::exception: " << e.what() << "\n";
        logFile.flush();
        logFile.close();

        std::cerr << "Press Enter to exit...\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return 1;
    }
    catch (...)
    {
        std::cerr << "FATAL unknown exception\n";
        logFile << "FATAL unknown exception\n";
        logFile.flush();
        logFile.close();

        std::cerr << "Press Enter to exit...\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return 1;
    }
}
