#include <GameEngine.h>
#include <Vec2.h>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>

#include "BarrelSpawner.h"
#include "BoatEntity.h"

using namespace Components;
using namespace Entity;
using namespace Systems;

// Define Screen Size
const int   SCREEN_WIDTH     = 1600;
const int   SCREEN_HEIGHT    = 1000;
const float GRAVITY_FORCE    = -10.0f;  // Box2D gravity (m/s²), negative = downward
const float PIXELS_PER_METER = 100.0f;  // Rendering scale: 100 pixels = 1 meter

// Derived playfield size in meters (screen dimensions divided by scale)
const float PLAYFIELD_WIDTH_METERS  = SCREEN_WIDTH / PIXELS_PER_METER;   // 16 m
const float PLAYFIELD_HEIGHT_METERS = SCREEN_HEIGHT / PIXELS_PER_METER;  // 10 m

const float BOUNDARY_THICKNESS_METERS = 0.5f;   // Thickness in meters
const float MAX_MUSIC_VOLUME          = 0.80f;  // 80% max volume
const float VOLUME_ADJUSTMENT_STEP    = 0.05f;  // Volume change per key press (5%)
const float INITIAL_VOLUME            = 0.15f;  // 15% initial volume

const size_t DEFAULT_BARREL_COUNT = 20;

class FishingGame
{
private:
    std::unique_ptr<GameEngine>     m_gameEngine;
    sf::Font                        m_font;
    bool                            m_running;
    bool                            m_fontLoaded;
    std::shared_ptr<Entity::Entity> m_oceanBackground;

    std::shared_ptr<Boat>                m_boat;
    std::vector<std::shared_ptr<Barrel>> m_barrels;
    std::unique_ptr<BarrelSpawner>       m_barrelSpawner;

    // Barrel spawn bounds (meters)
    float m_spawnMinX = BOUNDARY_THICKNESS_METERS;
    float m_spawnMaxX = PLAYFIELD_WIDTH_METERS - BOUNDARY_THICKNESS_METERS;
    float m_spawnMinY = BOUNDARY_THICKNESS_METERS;
    float m_spawnMaxY = PLAYFIELD_HEIGHT_METERS - BOUNDARY_THICKNESS_METERS;

    // Helper to get window from renderer
    sf::RenderWindow* getWindow() const
    {
        return m_gameEngine->getRenderer().getWindow();
    }

public:
    FishingGame() : m_running(true), m_fontLoaded(false), m_oceanBackground(nullptr)
    {
        // Create window configuration
        WindowConfig windowConfig;
        windowConfig.width      = SCREEN_WIDTH;
        windowConfig.height     = SCREEN_HEIGHT;
        windowConfig.title      = "Boat Example - ECS Rendering";
        windowConfig.vsync      = true;
        windowConfig.frameLimit = 60;

        // Initialize game engine with window config and particle scale
        m_gameEngine = std::make_unique<GameEngine>(windowConfig, Vec2(0.0f, 0.0f), 6, 1.0f / 60.0f, PIXELS_PER_METER);

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
        // Systems are now auto-initialized by GameEngine constructor
        // Just configure them as needed
        auto& audioSystem = m_gameEngine->getAudioSystem();

        // Set initial master volume
        std::cout << "Setting initial master volume to: " << INITIAL_VOLUME << std::endl;
        audioSystem.setMasterVolume(INITIAL_VOLUME);
        std::cout << "Master volume is now: " << audioSystem.getMasterVolume() << std::endl;

        // Load audio assets
        audioSystem.loadSound("background_music", "assets/audio/rainyday.mp3", AudioType::Music);
        audioSystem.loadSound("motor_boat", "assets/audio/motor_boat.mp3", AudioType::SFX);

        // Start background music
        audioSystem.playMusic("background_music", true, MAX_MUSIC_VOLUME);

        // Input Manager is already initialized by GameEngine - just disable ImGui passthrough
        m_gameEngine->getInputManager().setPassToImGui(false);

        // Set up Box2D physics world (gravity disabled)
        auto& physics = m_gameEngine->getPhysics();
        physics.setGravity({0.0f, 0.0f});

        // Particle system is already initialized - no need to call initialize again

        // Build world procedurally
        createOceanBackground();
        createBoundaryColliders();
        createBoatAndEffects();

        m_barrelSpawner = std::make_unique<BarrelSpawner>(*m_gameEngine, m_spawnMinX, m_spawnMaxX, m_spawnMinY, m_spawnMaxY);
        spawnBarrels(DEFAULT_BARREL_COUNT);

        // Entities are now initialized automatically when created - no manual update needed!

        std::cout << "Game initialized!" << std::endl;
        std::cout << "Physics: Box2D v3.1.1 (1 unit = 1 meter, Y-up)" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD            : Move player boat (W=forward, S=backward, A/D=turn when moving forward)" << std::endl;
        std::cout << "  Up/Down Arrow   : Adjust volume" << std::endl;
        std::cout << "  Escape          : Exit" << std::endl;
    }

    void createOceanBackground()
    {
        m_oceanBackground = m_gameEngine->spawn<Entity::Entity>("ocean");
        m_oceanBackground->addComponent<CTransform>(Vec2(8.0f, 5.0f), Vec2(1.0f, 1.0f), 0.0f);
        m_oceanBackground->addComponent<CRenderable>(VisualType::Rectangle, Color::Black, -10, true);

        auto* collider = m_oceanBackground->addComponent<CCollider2D>();
        collider->setIsSensor(false);
        collider->setDensity(1.0f);
        collider->setFriction(0.3f);
        collider->setRestitution(0.0f);
        collider->createBox(8.0f, 5.0f);
    }

    void createBoundaryColliders()
    {
        const struct BoundaryDef
        {
            std::string tag;
            Vec2        pos;
            float       halfWidth;
            float       halfHeight;
        } boundaries[] = {{"floor", Vec2(8.0f, 0.25f), 8.0f, 0.25f},
                          {"rightWall", Vec2(15.75f, 5.0f), 0.25f, 5.0f},
                          {"leftWall", Vec2(0.25f, 5.0f), 0.25f, 5.0f},
                          {"topWall", Vec2(8.0f, 9.75f), 8.0f, 0.25f}};

        for (const auto& def : boundaries)
        {
            auto boundary = m_gameEngine->spawn<Entity::Entity>(def.tag);
            boundary->addComponent<CTransform>(def.pos, Vec2(1.0f, 1.0f), 0.0f);

            auto* body = boundary->addComponent<CPhysicsBody2D>();
            body->setBodyType(BodyType::Static);
            body->setDensity(1.0f);
            body->setFriction(0.3f);
            body->setRestitution(0.15f);
            body->setLinearDamping(0.25f);
            body->setAngularDamping(0.1f);
            body->setGravityScale(1.0f);
            body->initialize(b2Vec2{def.pos.x, def.pos.y}, BodyType::Static);

            auto* collider = boundary->addComponent<CCollider2D>();
            collider->setIsSensor(false);
            collider->setDensity(1.0f);
            collider->setFriction(0.3f);
            collider->setRestitution(0.0f);
            collider->createBox(def.halfWidth, def.halfHeight);
        }
    }

    void createBoatAndEffects()
    {
        m_boat = m_gameEngine->spawn<Boat>("player", &m_gameEngine->getInputManager(), &m_gameEngine->getAudioSystem());
        // Boat and its child entities (emitters) are now fully initialized
    }

    void spawnBarrels(size_t count)
    {
        if (!m_barrelSpawner)
            return;
        m_barrels = m_barrelSpawner->spawn(count);
    }

    void update(float dt)
    {
        // Update Input Manager
        m_gameEngine->getInputManager().update(dt);

        // Handle window controls and key actions via SInputManager (prevents double polling)
        const auto& im = m_gameEngine->getInputManager();

        // Check for mouse clicks using abstracted MouseButton enum
        if (im.wasMouseReleased(MouseButton::Left))
        {
            sf::Vector2i mousePos = im.getMousePositionWindow();
            std::cout << "Left Mouse Button Release At: (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
        }
        if (im.wasMouseReleased(MouseButton::Right))
        {
            sf::Vector2i mousePos = im.getMousePositionWindow();
            std::cout << "Right Mouse Button Release At: (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
        }

        if (im.wasKeyPressed(KeyCode::Escape))
        {
            m_running = false;
        }
        if (im.wasKeyPressed(KeyCode::Up))
        {
            auto& audioSystem   = m_gameEngine->getAudioSystem();
            float currentVolume = audioSystem.getMasterVolume();
            float newVolume     = std::min(currentVolume + VOLUME_ADJUSTMENT_STEP, 1.0f);
            audioSystem.setMasterVolume(newVolume);
            std::cout << "Master Volume: " << static_cast<int>(newVolume * 100.0f) << "%" << std::endl;
        }
        if (im.wasKeyPressed(KeyCode::Down))
        {
            auto& audioSystem   = m_gameEngine->getAudioSystem();
            float currentVolume = audioSystem.getMasterVolume();
            float newVolume     = std::max(currentVolume - VOLUME_ADJUSTMENT_STEP, 0.0f);
            audioSystem.setMasterVolume(newVolume);
            std::cout << "Master Volume: " << static_cast<int>(newVolume * 100.0f) << "%" << std::endl;
        }

        // Update Box2D physics
        m_gameEngine->getPhysics().update(dt);

        // Update particle system
        m_gameEngine->getParticleSystem().update(dt);

        // Update Audio System
        m_gameEngine->getAudioSystem().update(dt);

        // Update EntityManager
        m_gameEngine->getEntityManager().update(dt);
    }

    void render()
    {
        auto* window = getWindow();
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
        auto* window = getWindow();
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
