#include <GameEngine.h>

#include <SFML/Graphics.hpp>

#include <Color.h>
#include <Components.h>

#include <exception>
#include <iostream>
#include <vector>

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

static Entity createBoat(World& world)
{
    // Tuned values copied from the Example on `master`.
    static constexpr float kBoatPosX = 9.20209f;
    static constexpr float kBoatPosY = 7.90827f;
    static constexpr float kBoatRot  = 1.73084f;

    static constexpr float kBoatLinearDamping  = 0.75f;
    static constexpr float kBoatAngularDamping = 0.75f;
    static constexpr float kBoatGravityScale   = 1.0f;

    static constexpr float kBoatColliderDensity     = 5.0f;
    static constexpr float kBoatColliderFriction    = 0.5f;
    static constexpr float kBoatColliderRestitution = 0.125f;

    static const std::vector<std::vector<Vec2>> kBoatHullFixtures = {
        {{0.225f, 0.0f}, {-0.225f, 0.0f}, {-0.225f, -0.0875f}, {-0.1575f, -0.39375f}, {0.1575f, -0.39375f}, {0.225f, -0.0875f}},
        {{-0.225f, 0.0f}, {0.225f, 0.0f}, {0.223438f, 0.0401042f}, {-0.223438f, 0.0401042f}},
        {{-0.223438f, 0.0401042f}, {0.223438f, 0.0401042f}, {0.21875f, 0.0802083f}, {-0.21875f, 0.0802083f}},
        {{-0.21875f, 0.0802083f}, {0.21875f, 0.0802083f}, {0.210938f, 0.120313f}, {-0.210938f, 0.120313f}},
        {{-0.210938f, 0.120313f}, {0.210938f, 0.120313f}, {0.2f, 0.160417f}, {-0.2f, 0.160417f}},
        {{-0.2f, 0.160417f}, {0.2f, 0.160417f}, {0.185937f, 0.200521f}, {-0.185937f, 0.200521f}},
        {{-0.185937f, 0.200521f}, {0.185937f, 0.200521f}, {0.16875f, 0.240625f}, {-0.16875f, 0.240625f}},
        {{-0.16875f, 0.240625f}, {0.16875f, 0.240625f}, {0.148438f, 0.280729f}, {-0.148438f, 0.280729f}},
        {{-0.148438f, 0.280729f}, {0.148438f, 0.280729f}, {0.125f, 0.320833f}, {-0.125f, 0.320833f}},
        {{-0.125f, 0.320833f}, {0.125f, 0.320833f}, {0.0984375f, 0.360938f}, {-0.0984375f, 0.360938f}},
        {{-0.0984375f, 0.360938f}, {0.0984375f, 0.360938f}, {0.06875f, 0.401042f}, {-0.06875f, 0.401042f}},
        {{-0.06875f, 0.401042f}, {0.06875f, 0.401042f}, {0.0359375f, 0.441146f}, {-0.0359375f, 0.441146f}},
        {{-0.0359375f, 0.441146f}, {0.0359375f, 0.441146f}, {0.0f, 0.48125f}},
    };

    Entity boat = world.createEntity();

    world.components().add<Components::CTransform>(boat, Vec2{kBoatPosX, kBoatPosY}, Vec2{1.0f, 1.0f}, kBoatRot);
    world.components().add<Components::CTexture>(boat, "assets/textures/boat.png");
    world.components().add<Components::CRenderable>(boat, Components::VisualType::Sprite, Color::White, 10, true);
    world.components().add<Components::CMaterial>(boat, Color::White, Components::BlendMode::Alpha, 1.0f);

    auto* body           = world.components().add<Components::CPhysicsBody2D>(boat);
    body->bodyType       = Components::BodyType::Dynamic;
    body->fixedRotation  = false;
    body->linearDamping  = kBoatLinearDamping;
    body->angularDamping = kBoatAngularDamping;
    body->gravityScale   = kBoatGravityScale;

    auto* collider        = world.components().add<Components::CCollider2D>(boat);
    collider->sensor      = false;
    collider->density     = kBoatColliderDensity;
    collider->friction    = kBoatColliderFriction;
    collider->restitution = kBoatColliderRestitution;
    if (!kBoatHullFixtures.empty())
    {
        collider->createPolygon(kBoatHullFixtures.front(), 0.02f);
        for (size_t i = 1; i < kBoatHullFixtures.size(); ++i)
        {
            collider->addPolygon(kBoatHullFixtures[i], 0.02f);
        }
    }

    world.components().add<Components::CInputController>(boat);

    auto* script = world.components().add<Components::CNativeScript>(boat);
    script->bind<Example::BoatScript>();

    return boat;
}

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
        (void)createBoat(world);
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
