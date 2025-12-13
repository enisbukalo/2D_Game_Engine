#include <GameEngine.h>

#include <SFML/Graphics.hpp>

#include <Color.h>
#include <Components.h>

#include <exception>
#include <iostream>

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
    try
    {
        WindowConfig windowConfig;
        windowConfig.width      = SCREEN_WIDTH;
        windowConfig.height     = SCREEN_HEIGHT;
        windowConfig.title      = "Boat Example - ECS Framework";
        windowConfig.vsync      = true;
        windowConfig.frameLimit = 144;

        GameEngine engine(windowConfig, GRAVITY);

        // Input Manager is already initialized by GameEngine - just disable ImGui passthrough
        engine.getInputManager().setPassToImGui(false);

        // Set up Box2D physics world (gravity disabled)
        engine.getPhysics().setGravity({0.0f, 0.0f});

        World& world = engine.world();
        (void)createAudioManager(world);
        (void)Example::spawnBoat(world);
        (void)createBarrelSpawner(world);

        std::cout << "Game initialized!\n";
        std::cout << "Physics: Box2D v3.1.1 (1 unit = 1 meter, Y-up)\n";

        sf::Clock clock;
        clock.restart();

        auto* window = engine.getRenderer().getWindow();
        while (engine.is_running() && window && window->isOpen())
        {
            const float dt = clock.restart().asSeconds();
            engine.update(dt);
            engine.render();
        }

        if (window)
        {
            window->close();
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "FATAL std::exception: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "FATAL unknown exception\n";
        return 1;
    }
}
