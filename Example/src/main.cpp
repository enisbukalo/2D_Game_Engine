#include <AudioTypes.h>
#include <CCollider2D.h>
#include <CInputController.h>
#include <CMaterial.h>
#include <CPhysicsBody2D.h>
#include <CRenderable.h>
#include <CTexture.h>
#include <CTransform.h>
#include <Color.h>
#include <Entity.h>
#include <GameEngine.h>
#include <Input/MouseButton.h>
#include <Vec2.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <sstream>

// Define Screen Size
const int   SCREEN_WIDTH              = 1600;
const int   SCREEN_HEIGHT             = 1000;
const int   INITIAL_BARREL_COUNT      = 100;
const bool  INITIAL_GRAVITY_ENABLED   = false;
const float TIME_STEP                 = 1.0f / 60.0f;  // 60 FPS
const float GRAVITY_FORCE             = -10.0f;        // Box2D gravity (m/s²), negative = downward
const float PIXELS_PER_METER          = 100.0f;        // Rendering scale: 100 pixels = 1 meter
const float RESTITUTION               = 0.5f;   // Bounciness factor (lowered from 0.8 to reduce collision energy)
const float BARREL_RADIUS_METERS      = 0.15f;  // Barrel radius in meters
const float BOUNDARY_THICKNESS_METERS = 0.5f;   // Thickness in meters
const float RANDOM_VELOCITY_RANGE     = 2.0f;   // Random velocity range: -2 to +2 m/s
const float PLAYER_SIZE_METERS        = 0.25f;  // Player square half-width/height in meters
const float PLAYER_FORCE              = 5.0f;   // Force applied for player movement
const float PLAYER_TURNING_FORCE      = 0.5f;   // Torque applied for player rotation
const float MOTOR_FADE_DURATION       = 2.0f;   // 2 second fade-in & fade-out
const float MOTOR_MAX_VOLUME          = 0.45f;  // 45% max volume
const float MAX_MUSIC_VOLUME          = 0.80f;  // 80% max volume
const float VOLUME_ADJUSTMENT_STEP    = 0.05f;  // Volume change per key press (5%)
const float INITIAL_VOLUME            = 0.15f;  // 15% initial volume

class BounceGame
{
private:
    std::unique_ptr<GameEngine> m_gameEngine;
    sf::Font                    m_font;
    int                         m_barrelAmount;
    bool                        m_running;
    bool                        m_fontLoaded;
    bool                        m_gravityEnabled;
    bool                        m_showColliders;
    bool                        m_showVectors;
    CPhysicsBody2D*             m_playerPhysics;
    std::shared_ptr<Entity>     m_player;

    // Audio state
    AudioHandle m_motorBoatHandle;

    // Helper function to convert meters to pixels for rendering (used for debug visualization)
    sf::Vector2f metersToPixels(const Vec2& meters) const
    {
        // Note: Y-flip for screen coordinates (Box2D Y-up -> Screen Y-down)
        return sf::Vector2f(meters.x * PIXELS_PER_METER, SCREEN_HEIGHT - (meters.y * PIXELS_PER_METER));
    }

    // Helper to get window from renderer
    sf::RenderWindow* getWindow() const
    {
        return m_gameEngine->getRenderer().getWindow();
    }

    // Helper function to generate random velocity in a symmetric range
    Vec2 getRandomVelocity() const
    {
        float velX = static_cast<float>((rand() % static_cast<int>(RANDOM_VELOCITY_RANGE * 2000 + 1)) - RANDOM_VELOCITY_RANGE * 1000)
                     / 1000.0f;
        float velY = static_cast<float>((rand() % static_cast<int>(RANDOM_VELOCITY_RANGE * 2000 + 1)) - RANDOM_VELOCITY_RANGE * 1000)
                     / 1000.0f;
        return Vec2(velX, velY);
    }

public:
    BounceGame()
        : m_running(true),
          m_barrelAmount(INITIAL_BARREL_COUNT),
          m_fontLoaded(false),
          m_gravityEnabled(INITIAL_GRAVITY_ENABLED),
          m_showColliders(false),
          m_showVectors(false),
          m_player(nullptr),
          m_playerPhysics(nullptr),
          m_motorBoatHandle(AudioHandle::invalid())
    {
        // Create window configuration
        WindowConfig windowConfig;
        windowConfig.width = SCREEN_WIDTH;
        windowConfig.height = SCREEN_HEIGHT;
        windowConfig.title = "Bouncing Barrels Example - ECS Rendering";
        windowConfig.vsync = true;
        windowConfig.frameLimit = 60;

        // Initialize game engine with window config
        m_gameEngine = std::make_unique<GameEngine>(windowConfig, Vec2(0.0f, GRAVITY_FORCE));

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

    ~BounceGame()
    {
        // Cleanup audio
        m_gameEngine->getAudioSystem().shutdown();
    }

    void init()
    {
        // Initialize audio system
        auto& audioSystem = m_gameEngine->getAudioSystem();
        audioSystem.initialize();

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

        // Set up Box2D physics world
        auto& physics = m_gameEngine->getPhysics();
        physics.setGravity({0.0f, m_gravityEnabled ? GRAVITY_FORCE : 0.0f});

        createBoundaryColliders();
        createPlayer();
        createBarrels();

        // Force EntityManager to process pending entities
        m_gameEngine->getEntityManager().update(0.0f);

        std::cout << "Game initialized!" << std::endl;
        std::cout << "Physics: Box2D v3.1.1 (1 unit = 1 meter, Y-up)" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD            : Move player boat (W=forward, S=backward, A/D=turn)" << std::endl;
        std::cout << "  Left/Right      : Adjust barrel count" << std::endl;
        std::cout << "  R               : Restart scenario" << std::endl;
        std::cout << "  G               : Toggle gravity" << std::endl;
        std::cout << "  C               : Toggle collider visibility" << std::endl;
        std::cout << "  V               : Toggle vector visualization" << std::endl;
        std::cout << "  Escape          : Exit" << std::endl;
        std::cout << "Number of barrels:" << m_barrelAmount << std::endl;
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;
    }

    void createBoundaryColliders()
    {
        // Screen dimensions in meters
        const float screenWidthMeters  = SCREEN_WIDTH / PIXELS_PER_METER;
        const float screenHeightMeters = SCREEN_HEIGHT / PIXELS_PER_METER;

        // Create boundary collider entities
        auto floor     = m_gameEngine->getEntityManager().addEntity("floor");
        auto rightWall = m_gameEngine->getEntityManager().addEntity("rightWall");
        auto leftWall  = m_gameEngine->getEntityManager().addEntity("leftWall");
        auto topWall   = m_gameEngine->getEntityManager().addEntity("topWall");

        // Position the boundary colliders in meters (Box2D Y-up coordinates)
        floor->addComponent<CTransform>(Vec2(screenWidthMeters / 2.0f, BOUNDARY_THICKNESS_METERS / 2.0f), Vec2(1.0f, 1.0f), 0.0f);
        rightWall->addComponent<CTransform>(Vec2(screenWidthMeters - BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f),
                                            Vec2(1.0f, 1.0f),
                                            0.0f);
        leftWall->addComponent<CTransform>(Vec2(BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f), Vec2(1.0f, 1.0f), 0.0f);
        topWall->addComponent<CTransform>(Vec2(screenWidthMeters / 2.0f, screenHeightMeters - BOUNDARY_THICKNESS_METERS / 2.0f),
                                          Vec2(1.0f, 1.0f),
                                          0.0f);

        // Create physics bodies and colliders
        auto* floorBody = floor->addComponent<CPhysicsBody2D>();
        floorBody->initialize({screenWidthMeters / 2.0f, BOUNDARY_THICKNESS_METERS / 2.0f}, BodyType::Static);
        floor->addComponent<CCollider2D>()->createBox(screenWidthMeters / 2.0f, BOUNDARY_THICKNESS_METERS / 2.0f);

        auto* rightBody = rightWall->addComponent<CPhysicsBody2D>();
        rightBody->initialize({screenWidthMeters - BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f}, BodyType::Static);
        rightWall->addComponent<CCollider2D>()->createBox(BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f);

        auto* leftBody = leftWall->addComponent<CPhysicsBody2D>();
        leftBody->initialize({BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f}, BodyType::Static);
        leftWall->addComponent<CCollider2D>()->createBox(BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f);

        auto* topBody = topWall->addComponent<CPhysicsBody2D>();
        topBody->initialize({screenWidthMeters / 2.0f, screenHeightMeters - BOUNDARY_THICKNESS_METERS / 2.0f}, BodyType::Static);
        topWall->addComponent<CCollider2D>()->createBox(screenWidthMeters / 2.0f, BOUNDARY_THICKNESS_METERS / 2.0f);
    }

    void createPlayer()
    {
        // Screen dimensions in meters
        const float screenWidthMeters  = SCREEN_WIDTH / PIXELS_PER_METER;
        const float screenHeightMeters = SCREEN_HEIGHT / PIXELS_PER_METER;

        // Calculate center position
        const float centerX = screenWidthMeters / 2.0f;
        const float centerY = screenHeightMeters / 2.0f;

        // Create player entity
        m_player = m_gameEngine->getEntityManager().addEntity("player");
        
        // Boat dimensions for proper sprite scaling
        const float boatLength = PLAYER_SIZE_METERS * 3.5f;  // 0.875 meters
        const float boatWidth  = PLAYER_SIZE_METERS * 1.8f;  // 0.45 meters
        
        // Scale factor to make sprite match collider size
        const float boatSpriteScale = 1.0f;  // Use 1.0 for natural size matching
        
        m_player->addComponent<CTransform>(Vec2(centerX, centerY), Vec2(boatSpriteScale, boatSpriteScale), 0.0f);

        // Add physics body
        m_playerPhysics = m_player->addComponent<CPhysicsBody2D>();
        m_playerPhysics->initialize({centerX, centerY}, BodyType::Dynamic);
        m_playerPhysics->setAngularDamping(0.75f);  // Damping to reduce spin over time
        m_playerPhysics->setLinearDamping(0.75f);   // Some linear damping for better control

        // Create boat shape with curved bow using multiple polygon segments
        // Boat points from stern (back) to bow (front), with Y-axis as forward
        // const float boatLength and boatWidth already defined above
        
        // Create single collider that will hold multiple polygon fixtures
        auto* collider = m_player->addComponent<CCollider2D>();

        // 1. Create main hull body (trapezoidal section narrowing toward stern)
        {
            std::vector<b2Vec2> hullVertices;
            hullVertices.push_back({-boatWidth * 0.35f, -boatLength * 0.45f});  // Back left (narrower)
            hullVertices.push_back({boatWidth * 0.35f, -boatLength * 0.45f});   // Back right (narrower)
            hullVertices.push_back({boatWidth * 0.5f, -boatLength * 0.1f});     // Mid-back right
            hullVertices.push_back({boatWidth * 0.5f, 0.0f});                   // Front right (widest)
            hullVertices.push_back({-boatWidth * 0.5f, 0.0f});                  // Front left (widest)
            hullVertices.push_back({-boatWidth * 0.5f, -boatLength * 0.1f});    // Mid-back left

            collider->createPolygon(hullVertices.data(), hullVertices.size(), 0.02f);
            collider->setRestitution(0.125f);
            collider->setDensity(5.0f);
            collider->setFriction(0.5f);
        }

        // 2. Create curved bow using multiple small polygon segments
        // More segments = smoother curve
        const int   numBowSegments = 12;                  // Increased for smoother curve
        const float bowLength      = boatLength * 0.55f;  // Length of bow section

        // Create bow segments arranged in a smooth parabolic curve
        for (int i = 0; i < numBowSegments; ++i)
        {
            // Calculate normalized position along the bow (0 to 1)
            float t1 = static_cast<float>(i) / static_cast<float>(numBowSegments);
            float t2 = static_cast<float>(i + 1) / static_cast<float>(numBowSegments);

            // Use parabolic curve for natural boat bow shape
            // Width decreases more gradually at first, then rapidly near the tip
            float width1 = boatWidth * 0.5f * (1.0f - t1 * t1);  // Parabolic taper
            float width2 = boatWidth * 0.5f * (1.0f - t2 * t2);

            float y1 = t1 * bowLength;
            float y2 = t2 * bowLength;

            // Create quad segments for smoother appearance
            std::vector<b2Vec2> sliceVertices;
            sliceVertices.push_back({-width1, y1});  // Left side at segment start
            sliceVertices.push_back({width1, y1});   // Right side at segment start
            sliceVertices.push_back({width2, y2});   // Right side at segment end
            sliceVertices.push_back({-width2, y2});  // Left side at segment end

            // Add this polygon as an additional fixture
            collider->addPolygon(sliceVertices.data(), sliceVertices.size(), 0.02f);
        }

        // Add rendering components for player boat texture
        auto boatTexture = m_player->addComponent<CTexture>("assets/textures/boat.png");
        auto boatRenderable = m_player->addComponent<CRenderable>(
            VisualType::Sprite,
            Color::White,
            10  // Higher z-index so boat renders on top of barrels
        );
        boatRenderable->setVisible(true);

        // Add material for the boat
        auto boatMaterial = m_player->addComponent<CMaterial>();
        boatMaterial->setTextureGuid(boatTexture->getGuid());
        boatMaterial->setTint(Color::White);
        boatMaterial->setOpacity(1.0f);
        boatMaterial->setBlendMode(BlendMode::Alpha);

        // Add input controller with action bindings
        auto* inputController = m_player->addComponent<CInputController>();

        // Bind movement actions
        ActionBinding moveForwardBinding;
        moveForwardBinding.keys.push_back(KeyCode::W);
        moveForwardBinding.trigger = ActionTrigger::Held;
        inputController->bindAction("MoveForward", moveForwardBinding);

        ActionBinding moveBackwardBinding;
        moveBackwardBinding.keys.push_back(KeyCode::S);
        moveBackwardBinding.trigger = ActionTrigger::Held;
        inputController->bindAction("MoveBackward", moveBackwardBinding);

        ActionBinding rotateLeftBinding;
        rotateLeftBinding.keys.push_back(KeyCode::A);
        rotateLeftBinding.trigger = ActionTrigger::Held;
        inputController->bindAction("RotateLeft", rotateLeftBinding);

        ActionBinding rotateRightBinding;
        rotateRightBinding.keys.push_back(KeyCode::D);
        rotateRightBinding.trigger = ActionTrigger::Held;
        inputController->bindAction("RotateRight", rotateRightBinding);

        // Set callbacks for movement actions
        inputController->setActionCallback("MoveForward",
                                           [this](ActionState state)
                                           {
                                               if ((state == ActionState::Held || state == ActionState::Pressed)
                                                   && m_playerPhysics && m_playerPhysics->isInitialized())
                                               {
                                                   // Get the forward direction based on current rotation
                                                   b2Vec2 forward = m_playerPhysics->getForwardVector();
                                                   b2Vec2 force = {forward.x * PLAYER_FORCE, forward.y * PLAYER_FORCE};
                                                   m_playerPhysics->applyForceToCenter(force);
                                                   startMotorBoat();
                                               }
                                               else if (state == ActionState::Released)
                                               {
                                                   checkStopMotorBoat();
                                               }
                                           });

        inputController->setActionCallback("MoveBackward",
                                           [this](ActionState state)
                                           {
                                               if ((state == ActionState::Held || state == ActionState::Pressed)
                                                   && m_playerPhysics && m_playerPhysics->isInitialized())
                                               {
                                                   // Get the forward direction and move backward (negative)
                                                   b2Vec2 forward = m_playerPhysics->getForwardVector();
                                                   b2Vec2 force = {-forward.x * PLAYER_FORCE, -forward.y * PLAYER_FORCE};
                                                   m_playerPhysics->applyForceToCenter(force);
                                                   startMotorBoat();
                                               }
                                               else if (state == ActionState::Released)
                                               {
                                                   checkStopMotorBoat();
                                               }
                                           });

        inputController->setActionCallback("RotateLeft",
                                           [this](ActionState state)
                                           {
                                               if ((state == ActionState::Held || state == ActionState::Pressed)
                                                   && m_playerPhysics && m_playerPhysics->isInitialized())
                                               {
                                                   m_playerPhysics->applyTorque(PLAYER_TURNING_FORCE);
                                               }
                                           });

        inputController->setActionCallback("RotateRight",
                                           [this](ActionState state)
                                           {
                                               if ((state == ActionState::Held || state == ActionState::Pressed)
                                                   && m_playerPhysics && m_playerPhysics->isInitialized())
                                               {
                                                   m_playerPhysics->applyTorque(-PLAYER_TURNING_FORCE);
                                               }
                                           });
    }

    void createBarrels()
    {
        // Screen dimensions in meters
        const float screenWidthMeters  = SCREEN_WIDTH / PIXELS_PER_METER;
        const float screenHeightMeters = SCREEN_HEIGHT / PIXELS_PER_METER;

        // Calculate spawn boundaries accounting for boundary thickness and barrel radius
        const float MIN_X = BOUNDARY_THICKNESS_METERS + BARREL_RADIUS_METERS;
        const float MAX_X = screenWidthMeters - BOUNDARY_THICKNESS_METERS - BARREL_RADIUS_METERS;
        const float MIN_Y = BOUNDARY_THICKNESS_METERS + BARREL_RADIUS_METERS;
        const float MAX_Y = screenHeightMeters - BOUNDARY_THICKNESS_METERS - BARREL_RADIUS_METERS;

        for (int i = 0; i < m_barrelAmount; i++)
        {
            // Random position within safe spawn area (in meters)
            float randomX = MIN_X + static_cast<float>(rand()) / RAND_MAX * (MAX_X - MIN_X);
            float randomY = MIN_Y + static_cast<float>(rand()) / RAND_MAX * (MAX_Y - MIN_Y);

            auto barrel = m_gameEngine->getEntityManager().addEntity("barrel");
            barrel->addComponent<CTransform>(Vec2(randomX, randomY), Vec2(1.0f, 1.0f), 0.0f);

            auto* physicsBody = barrel->addComponent<CPhysicsBody2D>();
            physicsBody->initialize({randomX, randomY}, BodyType::Dynamic);

            auto* collider = barrel->addComponent<CCollider2D>();
            collider->createCircle(BARREL_RADIUS_METERS);
            collider->setRestitution(RESTITUTION);
            collider->setDensity(1.0f);  // Set barrel density (lighter than player)

            // Add damping to gradually reduce velocity (prevents barrels from maintaining excessive speeds)
            physicsBody->setLinearDamping(0.125f);

            // Randomize initial velocity
            physicsBody->setLinearVelocity({getRandomVelocity().x, getRandomVelocity().y});

            // Add rendering components for barrel sprite
            auto barrelTexture = barrel->addComponent<CTexture>("assets/textures/barrel.png");
            auto barrelRenderable = barrel->addComponent<CRenderable>(
                VisualType::Sprite,
                Color::White,
                0  // Lower z-index so barrels render behind boat
            );
            barrelRenderable->setVisible(true);

            // Add material for the barrel
            auto barrelMaterial = barrel->addComponent<CMaterial>();
            barrelMaterial->setTextureGuid(barrelTexture->getGuid());
            barrelMaterial->setTint(Color::White);
            barrelMaterial->setOpacity(1.0f);
            barrelMaterial->setBlendMode(BlendMode::Alpha);
        }
    }

    void toggleGravity()
    {
        m_gravityEnabled = !m_gravityEnabled;

        // Update global gravity in Box2D physics system
        auto& physics = m_gameEngine->getPhysics();
        if (m_gravityEnabled)
        {
            physics.setGravity({0.0f, GRAVITY_FORCE});
        }
        else
        {
            physics.setGravity({0.0f, 0.0f});
        }
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;
    }

    void toggleColliders()
    {
        m_showColliders = !m_showColliders;
        std::cout << "Colliders: " << (m_showColliders ? "ON" : "OFF") << std::endl;
    }

    void toggleVectors()
    {
        m_showVectors = !m_showVectors;
        std::cout << "Vectors: " << (m_showVectors ? "ON" : "OFF") << std::endl;
    }

    void startMotorBoat()
    {
        auto& audioSystem = m_gameEngine->getAudioSystem();

        // Check if motor boat is already playing
        if (audioSystem.isPlayingSFX(m_motorBoatHandle))
        {
            // If fading out, fade back in to target volume
            FadeConfig fadeIn = FadeConfig::linear(MOTOR_FADE_DURATION, true);
            audioSystem.fadeSFX(m_motorBoatHandle, MOTOR_MAX_VOLUME, fadeIn);
            return;
        }

        // Start motor boat with fade-in
        FadeConfig fadeIn = FadeConfig::linear(MOTOR_FADE_DURATION, true);
        m_motorBoatHandle = audioSystem.playSFXWithFade("motor_boat", MOTOR_MAX_VOLUME, 1.0f, true, fadeIn);
    }

    void checkStopMotorBoat()
    {
        // Check if any movement key is still being held
        const auto& inputManager       = m_gameEngine->getInputManager();
        bool        anyMovementKeyHeld = inputManager.isKeyDown(KeyCode::W) || inputManager.isKeyDown(KeyCode::S);

        if (!anyMovementKeyHeld && m_gameEngine->getAudioSystem().isPlayingSFX(m_motorBoatHandle))
        {
            // Stop with fade-out
            FadeConfig fadeOut = FadeConfig::linear(MOTOR_FADE_DURATION, true);
            m_gameEngine->getAudioSystem().stopSFXWithFade(m_motorBoatHandle, fadeOut);
        }
    }

    void spawnRandomBarrel()
    {
        // Screen dimensions in meters
        const float screenWidthMeters  = SCREEN_WIDTH / PIXELS_PER_METER;
        const float screenHeightMeters = SCREEN_HEIGHT / PIXELS_PER_METER;

        // Calculate spawn boundaries
        const float MIN_X = BOUNDARY_THICKNESS_METERS + BARREL_RADIUS_METERS;
        const float MAX_X = screenWidthMeters - BOUNDARY_THICKNESS_METERS - BARREL_RADIUS_METERS;
        const float MIN_Y = BOUNDARY_THICKNESS_METERS + BARREL_RADIUS_METERS;
        const float MAX_Y = screenHeightMeters - BOUNDARY_THICKNESS_METERS - BARREL_RADIUS_METERS;

        // Random position within safe spawn area (in meters)
        float randomX = MIN_X + static_cast<float>(rand()) / RAND_MAX * (MAX_X - MIN_X);
        float randomY = MIN_Y + static_cast<float>(rand()) / RAND_MAX * (MAX_Y - MIN_Y);

        auto barrel = m_gameEngine->getEntityManager().addEntity("barrel");
        barrel->addComponent<CTransform>(Vec2(randomX, randomY), Vec2(1.0f, 1.0f), 0.0f);

        auto* physicsBody = barrel->addComponent<CPhysicsBody2D>();
        physicsBody->initialize({randomX, randomY}, BodyType::Dynamic);

        auto* collider = barrel->addComponent<CCollider2D>();
        collider->createCircle(BARREL_RADIUS_METERS);
        collider->setRestitution(RESTITUTION);
        collider->setDensity(1.0f);  // Set barrel density (lighter than player)

        // Add damping to gradually reduce velocity (prevents barrels from maintaining excessive speeds)
        physicsBody->setLinearDamping(0.2f);

        // Randomize initial velocity
        physicsBody->setLinearVelocity({getRandomVelocity().x, getRandomVelocity().y});

        // Add rendering components for barrel sprite
        auto barrelTexture = barrel->addComponent<CTexture>("assets/textures/barrel.png");
        auto barrelRenderable = barrel->addComponent<CRenderable>(
            VisualType::Sprite,
            Color::White,
            0  // Lower z-index so barrels render behind boat
        );
        barrelRenderable->setVisible(true);

        // Add material for the barrel
        auto barrelMaterial = barrel->addComponent<CMaterial>();
        barrelMaterial->setTextureGuid(barrelTexture->getGuid());
        barrelMaterial->setTint(Color::White);
        barrelMaterial->setOpacity(1.0f);
        barrelMaterial->setBlendMode(BlendMode::Alpha);
    }

    void removeRandomBarrel()
    {
        auto barrels = m_gameEngine->getEntityManager().getEntitiesByTag("barrel");
        if (!barrels.empty())
        {
            // Pick a random barrel to remove
            int randomIndex = rand() % barrels.size();
            barrels[randomIndex]->destroy();
        }
    }

    void restart()
    {
        std::cout << "\n=== Restarting scenario ===" << std::endl;
        std::cout << "Barrel count:" << m_barrelAmount << std::endl;
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;

        // Stop motor boat if playing
        auto& audioSystem = m_gameEngine->getAudioSystem();
        if (audioSystem.isPlayingSFX(m_motorBoatHandle))
        {
            audioSystem.stopSFX(m_motorBoatHandle);
            m_motorBoatHandle = AudioHandle::invalid();
        }

        // Clear all entities
        m_gameEngine->getEntityManager().clear();

        // Reset physics world
        auto& physics = m_gameEngine->getPhysics();
        physics.setGravity({0.0f, m_gravityEnabled ? GRAVITY_FORCE : 0.0f});

        // Recreate boundary colliders, player, and barrels
        createBoundaryColliders();
        createPlayer();
        createBarrels();

        // Force EntityManager to process pending entities
        m_gameEngine->getEntityManager().update(0.0f);

        std::cout << "=== Restart complete ===" << std::endl;
    }

    void drawVector(const Vec2& startMeters, const Vec2& vectorMeters, const sf::Color& color, float scale = 1.0f)
    {
        // Convert physics positions to screen coordinates
        sf::Vector2f startPixels = metersToPixels(startMeters);

        // For vectors, we only need to scale and flip Y direction
        Vec2 scaledVector = vectorMeters * scale;
        sf::Vector2f vectorPixels(scaledVector.x * PIXELS_PER_METER, -scaledVector.y * PIXELS_PER_METER);  // Negative Y for screen
        sf::Vector2f endPixels = startPixels + vectorPixels;

        // Draw line using VertexArray
        sf::VertexArray line(sf::Lines, 2);
        line[0].position = startPixels;
        line[0].color    = color;
        line[1].position = endPixels;
        line[1].color    = color;

        auto* window = getWindow();
        if (window)
        {
            window->draw(line);
        }
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
        if (im.wasKeyPressed(KeyCode::Left))
        {
            if (m_barrelAmount > 1)
            {
                m_barrelAmount--;
                removeRandomBarrel();
                std::cout << "Barrel count: " << m_barrelAmount << std::endl;
            }
        }
        if (im.wasKeyPressed(KeyCode::Right))
        {
            if (m_barrelAmount < 1000)
            {
                m_barrelAmount++;
                spawnRandomBarrel();
                std::cout << "Barrel count: " << m_barrelAmount << std::endl;
            }
        }
        if (im.wasKeyPressed(KeyCode::R))
        {
            restart();
        }
        if (im.wasKeyPressed(KeyCode::G))
        {
            toggleGravity();
        }
        if (im.wasKeyPressed(KeyCode::C))
        {
            toggleColliders();
        }
        if (im.wasKeyPressed(KeyCode::V))
        {
            toggleVectors();
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

        // Update Audio System
        m_gameEngine->getAudioSystem().update(dt);

        // Update EntityManager
        m_gameEngine->getEntityManager().update(dt);
    }

    void render()
    {
        auto* window = getWindow();
        if (!window) return;

        // Use GameEngine's render pipeline for ECS entities
        m_gameEngine->render();

        // Manual drawing for debug visualization (colliders, vectors, UI)
        // Draw boundary colliders
        std::vector<std::string> boundaryTags = {"floor", "rightWall", "leftWall", "topWall"};
        for (const auto& tag : boundaryTags)
        {
            auto boundaries = m_gameEngine->getEntityManager().getEntitiesByTag(tag);
            for (auto& boundaryCollider : boundaries)
            {
                if (!boundaryCollider->hasComponent<CTransform>() || !boundaryCollider->hasComponent<CCollider2D>())
                    continue;

                auto* transform = boundaryCollider->getComponent<CTransform>();
                auto* collider  = boundaryCollider->getComponent<CCollider2D>();

                Vec2         posMeters = transform->getPosition();
                sf::Vector2f posPixels = metersToPixels(posMeters);

                float halfWidth  = collider->getBoxHalfWidth() * PIXELS_PER_METER;
                float halfHeight = collider->getBoxHalfHeight() * PIXELS_PER_METER;

                sf::RectangleShape shape(sf::Vector2f(halfWidth * 2, halfHeight * 2));
                shape.setOrigin(halfWidth, halfHeight);
                shape.setPosition(posPixels);
                shape.setFillColor(sf::Color(100, 100, 100));  // Gray
                if (m_showColliders)
                {
                    shape.setOutlineColor(sf::Color(0, 255, 0));  // Lime green
                    shape.setOutlineThickness(2.0f);
                }
                window->draw(shape);
            }
        }

        // Barrels are now rendered by the ECS rendering pipeline automatically
        // Only draw debug colliders if enabled
        if (m_showColliders)
        {
            auto barrels = m_gameEngine->getEntityManager().getEntitiesByTag("barrel");
            for (auto& barrel : barrels)
            {
                if (!barrel->hasComponent<CTransform>() || !barrel->hasComponent<CCollider2D>())
                    continue;

                auto* transform = barrel->getComponent<CTransform>();
                auto* collider  = barrel->getComponent<CCollider2D>();

                Vec2         posMeters    = transform->getPosition();
                sf::Vector2f posPixels    = metersToPixels(posMeters);
                float        radiusPixels = collider->getCircleRadius() * PIXELS_PER_METER;

                sf::CircleShape colliderShape(radiusPixels);
                colliderShape.setOrigin(radiusPixels, radiusPixels);
                colliderShape.setPosition(posPixels);
                colliderShape.setFillColor(sf::Color::Transparent);
                colliderShape.setOutlineColor(sf::Color::Green);
                colliderShape.setOutlineThickness(2.0f);

                window->draw(colliderShape);
            }
        }

        // Draw player boat debug colliders
        if (m_showColliders)
        {
            auto players = m_gameEngine->getEntityManager().getEntitiesByTag("player");
            for (auto& player : players)
        {
            if (!player->hasComponent<CTransform>() || !player->hasComponent<CCollider2D>())
                continue;

            auto* transform = player->getComponent<CTransform>();
            auto* collider  = player->getComponent<CCollider2D>();

            Vec2         posMeters = transform->getPosition();
            sf::Vector2f posPixels = metersToPixels(posMeters);
            float        rotation  = transform->getRotation();  // Get rotation in radians

            // Draw all polygon fixtures in the collider
            const auto& fixtures = collider->getFixtures();
            for (size_t fixtureIdx = 0; fixtureIdx < fixtures.size(); ++fixtureIdx)
            {
                const auto& fixture = fixtures[fixtureIdx];

                if (fixture.shapeType == ColliderShape::Polygon)
                {
                    const b2Vec2* vertices    = collider->getPolygonVertices(fixtureIdx);
                    int           vertexCount = collider->getPolygonVertexCount(fixtureIdx);

                    if (vertices && vertexCount > 0)
                    {
                        sf::ConvexShape boatShape(vertexCount);

                        // Transform and set each vertex
                        for (int i = 0; i < vertexCount; ++i)
                        {
                            // Rotate vertex around origin
                            float cosR     = std::cos(rotation);
                            float sinR     = std::sin(rotation);
                            float rotatedX = vertices[i].x * cosR - vertices[i].y * sinR;
                            float rotatedY = vertices[i].x * sinR + vertices[i].y * cosR;

                            // Convert to pixels (note: Y-flip for screen coordinates)
                            float pixelX = rotatedX * PIXELS_PER_METER;
                            float pixelY = -rotatedY * PIXELS_PER_METER;  // Negative for screen Y-down

                            boatShape.setPoint(i, sf::Vector2f(pixelX, pixelY));
                        }

                        boatShape.setPosition(posPixels);
                        boatShape.setFillColor(sf::Color(200, 150, 100));  // Brownish color for boat
                        if (m_showColliders)
                        {
                            boatShape.setOutlineColor(sf::Color::Magenta);  // Magenta outline for player
                            boatShape.setOutlineThickness(3.0f);
                        }
                        window->draw(boatShape);
                    }
                }
                else if (fixture.shapeType == ColliderShape::Segment || fixture.shapeType == ColliderShape::ChainSegment)
                {
                    // Get segment endpoints
                    b2Vec2 p1, p2;
                    if (fixture.shapeType == ColliderShape::Segment)
                    {
                        p1 = fixture.shapeData.segment.point1;
                        p2 = fixture.shapeData.segment.point2;
                    }
                    else  // ChainSegment
                    {
                        p1 = fixture.shapeData.chainSegment.point1;
                        p2 = fixture.shapeData.chainSegment.point2;
                    }

                    // Rotate endpoints around origin
                    float cosR = std::cos(rotation);
                    float sinR = std::sin(rotation);

                    float rotatedX1 = p1.x * cosR - p1.y * sinR;
                    float rotatedY1 = p1.x * sinR + p1.y * cosR;
                    float rotatedX2 = p2.x * cosR - p2.y * sinR;
                    float rotatedY2 = p2.x * sinR + p2.y * cosR;

                    // Convert to pixels
                    float pixelX1 = rotatedX1 * PIXELS_PER_METER;
                    float pixelY1 = -rotatedY1 * PIXELS_PER_METER;
                    float pixelX2 = rotatedX2 * PIXELS_PER_METER;
                    float pixelY2 = -rotatedY2 * PIXELS_PER_METER;

                    // Draw line segment
                    sf::Vertex line[] = {sf::Vertex(sf::Vector2f(posPixels.x + pixelX1, posPixels.y + pixelY1),
                                                    sf::Color(200, 150, 100)),
                                         sf::Vertex(sf::Vector2f(posPixels.x + pixelX2, posPixels.y + pixelY2),
                                                    sf::Color(200, 150, 100))};
                    window->draw(line, 2, sf::Lines);

                    // Draw thicker line if colliders are shown
                    if (m_showColliders)
                    {
                        sf::Vertex thickLine[] = {sf::Vertex(sf::Vector2f(posPixels.x + pixelX1, posPixels.y + pixelY1),
                                                             sf::Color::Magenta),
                                                  sf::Vertex(sf::Vector2f(posPixels.x + pixelX2, posPixels.y + pixelY2),
                                                             sf::Color::Magenta)};
                        // Draw multiple times for thickness
                        for (int offset = -1; offset <= 1; ++offset)
                        {
                            thickLine[0].position.x += offset;
                            thickLine[1].position.x += offset;
                            window->draw(thickLine, 2, sf::Lines);
                            thickLine[0].position.y += offset;
                            thickLine[1].position.y += offset;
                            window->draw(thickLine, 2, sf::Lines);
                        }
                    }
                }
            }
        }
        }

        // Draw velocity vectors
        if (m_showVectors)
        {
            // Get all entities and check all entities have CTransform and CPhysicsBody2D
            auto player      = m_gameEngine->getEntityManager().getEntitiesByTag("player")[0];
            auto allBarrels = m_gameEngine->getEntityManager().getEntitiesByTag("barrel");

            // Combine barrels and players into one iterable list
            auto allEntities = allBarrels;
            allEntities.push_back(player);

            for (auto& entity : allEntities)
            {
                if (!entity->hasComponent<CTransform>() || !entity->hasComponent<CPhysicsBody2D>())
                    continue;

                auto* transform   = entity->getComponent<CTransform>();
                auto* physicsBody = entity->getComponent<CPhysicsBody2D>();

                Vec2 posMeters = transform->getPosition();

                // Draw velocity vector (yellow)
                b2Vec2 velocity = physicsBody->getLinearVelocity();
                Vec2   velocityMeters(velocity.x, velocity.y);
                if (velocityMeters.length() > 0.01f)  // Only draw if velocity is non-negligible
                {
                    drawVector(posMeters, velocityMeters, sf::Color::Yellow, 0.5f);
                }
            }
        }

        // Draw UI text showing current status
        if (m_fontLoaded)
        {
            const auto& audioSystem   = m_gameEngine->getAudioSystem();
            float       currentVolume = audioSystem.getMasterVolume();

            std::ostringstream oss;
            oss << "Box2D Physics (1 unit = 1 meter, Y-up) - ECS Rendering Pipeline\n";
            oss << "Barrel Count: " << m_barrelAmount << " (Use Left/Right to add/remove)\n";
            oss << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << " (Press G to toggle)\n";
            oss << "Colliders: " << (m_showColliders ? "ON" : "OFF") << " (Press C to toggle)\n";
            oss << "Vectors: " << (m_showVectors ? "ON" : "OFF") << " (Press V to toggle)\n";
            oss << "Master Volume: " << static_cast<int>(currentVolume * 100.0f) << "% (Use Up/Down to adjust)";

            sf::Text text;
            text.setFont(m_font);
            text.setString(oss.str());
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::White);
            text.setPosition(10.0f, 10.0f);
            window->draw(text);
        }

        // Display is handled by GameEngine's render() method
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
        BounceGame game;
        game.run();
    }
    catch (const std::exception& e)
    {
        std::cout << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
