#include <AudioTypes.h>
#include <CCollider2D.h>
#include <CInputController.h>
#include <CMaterial.h>
#include <CParticleEmitter.h>
#include <CPhysicsBody2D.h>
#include <CRenderable.h>
#include <CShader.h>
#include <CTexture.h>
#include <CTransform.h>
#include <Color.h>
#include <Entity.h>
#include <GameEngine.h>
#include <Input/MouseButton.h>
#include <SceneManager.h>
#include <Vec2.h>
#include <windows.h>
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <map>
#include <memory>
#include <sstream>

using namespace Components;
using namespace Entity;
using namespace Systems;

// Define Screen Size
const int   SCREEN_WIDTH            = 1600;
const int   SCREEN_HEIGHT           = 1000;
const bool  INITIAL_GRAVITY_ENABLED = false;
const float TIME_STEP               = 1.0f / 60.0f;  // 60 FPS
const float GRAVITY_FORCE           = -10.0f;        // Box2D gravity (m/s²), negative = downward
const float PIXELS_PER_METER        = 100.0f;        // Rendering scale: 100 pixels = 1 meter
const float RESTITUTION             = 0.5f;          // Bounciness factor (lowered from 0.8 to reduce collision energy)

// Rendering Constants
const int BOAT_INDEX         = 10;  // Z-index for boat rendering
const int BACKGROUND_INDEX   = 0;   // Z-index for background rendering
const int BUBBLE_TRAIL_INDEX = 5;   // Z-index for bubble trail rendering
const int HULL_SPRAY_INDEX   = 5;   // Z-index for hull spray rendering

const float BOUNDARY_THICKNESS_METERS = 0.5f;   // Thickness in meters
const float RANDOM_VELOCITY_RANGE     = 2.0f;   // Random velocity range: -2 to +2 m/s
const float PLAYER_SIZE_METERS        = 0.25f;  // Player square half-width/height in meters
const float PLAYER_FORCE              = 5.0f;   // Force applied for player movement
const float PLAYER_TURNING_FORCE      = 0.5f;   // Base torque/force multiplier for player rotation
const float RUDDER_OFFSET_METERS      = 0.35f;  // Distance from center to stern (meters) where rudder force is applied
const float RUDDER_FORCE_MULTIPLIER   = 1.0f;   // Multiplier for lateral rudder force
const float RUDDER_SMOOTH_K           = 0.18f;  // Smooth parameter to scale rudder effectiveness with speed (soft clamp)
const float MIN_SPEED_FOR_STEERING    = 0.15f; // Minimum speed (m/s) required for steering effectiveness (coasting)
const float RUDDER_MIN_EFFECTIVE_SCALE = 0.025f; // Minimum rudder effect scale applied at MIN_SPEED_FOR_STEERING (lowered for subtler low-speed turning)
const float MOTOR_FADE_DURATION       = 2.0f;   // 2 second fade-in & fade-out
const float MOTOR_MAX_VOLUME          = 0.45f;  // 45% max volume
const float MAX_MUSIC_VOLUME          = 0.80f;  // 80% max volume
const float VOLUME_ADJUSTMENT_STEP    = 0.05f;  // Volume change per key press (5%)
const float INITIAL_VOLUME            = 0.15f;  // 15% initial volume

// Scene file name (will be combined with getBasePath())
const std::string SCENE_FILE_NAME = "main_scene.json";

/**
 * @brief Gets the directory containing the executable
 * @return std::filesystem::path The path to the executable's directory
 */
std::filesystem::path getExecutableDir()
{
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

/**
 * @brief Gets the base path for the Example project directory
 *
 * This function finds the Example directory by looking for the saved_games
 * and assets folders, starting from the executable location and walking up.
 *
 * @return std::filesystem::path The path to the Example directory
 */
std::filesystem::path getBasePath()
{
    // Start from executable directory
    std::filesystem::path searchPath = getExecutableDir();

    // Walk up the directory tree looking for the Example directory
    // We identify it by the presence of saved_games AND assets folder together
    for (int i = 0; i < 10; ++i)  // Limit search depth
    {
        if (std::filesystem::exists(searchPath / "saved_games") && std::filesystem::exists(searchPath / "assets"))
        {
            return searchPath;
        }

        auto parentPath = searchPath.parent_path();
        if (parentPath == searchPath)
        {
            // Reached root, stop searching
            break;
        }
        searchPath = parentPath;
    }

    // Fallback: return executable directory's parent (assumes we're in build/)
    std::cerr << "WARNING: Could not find Example directory with saved_games and assets folders" << std::endl;
    return getExecutableDir().parent_path();
}

/**
 * @brief Gets the full path to the scene file
 * @return std::string The absolute path to the scene file
 */
std::string getSceneFilePath()
{
    return (getBasePath() / "saved_games" / SCENE_FILE_NAME).string();
}

/**
 * @brief Creates a backup of the scene file
 * @param filepath Path to the scene file to backup
 * @return true if backup was created successfully, false otherwise
 */
bool createSceneBackup(const std::string& filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        std::cout << "No scene file to backup: " << filepath << std::endl;
        return false;
    }

    try
    {
        std::filesystem::path originalPath(filepath);
        std::filesystem::path backupPath = originalPath.parent_path()
                                           / (originalPath.stem().string() + "_backup" + originalPath.extension().string());

        std::filesystem::copy_file(originalPath, backupPath, std::filesystem::copy_options::overwrite_existing);

        std::cout << "Scene backup created: " << backupPath.string() << std::endl;
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to create scene backup: " << e.what() << std::endl;
        return false;
    }
}

class FishingGame
{
private:
    std::unique_ptr<GameEngine>     m_gameEngine;
    sf::Font                        m_font;
    bool                            m_running;
    bool                            m_fontLoaded;
    bool                            m_gravityEnabled;
    bool                            m_showColliders;
    bool                            m_showVectors;
    CPhysicsBody2D*                 m_playerPhysics;
    std::shared_ptr<Entity::Entity> m_player;
    std::shared_ptr<Entity::Entity> m_oceanBackground;

    // Audio state
    AudioHandle m_motorBoatHandle;

    // Particle system
    sf::Texture                     m_bubbleTexture;
    sf::Texture                     m_sprayTexture;
    std::shared_ptr<Entity::Entity> m_bubbleTrailEntity = nullptr;  // Separate entity for bubble trail
    std::shared_ptr<Entity::Entity> m_hullSprayEntity   = nullptr;  // Separate entity for hull spray
    CParticleEmitter* m_hullSprayEmitter = nullptr;  // Reference to hull spray emitter for dynamic updates

    // Velocity visualization (entity -> velocity line entity mapping)
    std::map<Entity::Entity*, std::shared_ptr<Entity::Entity>> m_velocityLines;

    // Scene loading state
    bool m_sceneLoaded = false;

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

public:
    FishingGame()
        : m_running(true),
          m_fontLoaded(false),
          m_gravityEnabled(INITIAL_GRAVITY_ENABLED),
          m_showColliders(false),
          m_showVectors(false),
          m_player(nullptr),
          m_playerPhysics(nullptr),
          m_oceanBackground(nullptr),
          m_motorBoatHandle(AudioHandle::invalid())
    {
        // Create window configuration
        WindowConfig windowConfig;
        windowConfig.width      = SCREEN_WIDTH;
        windowConfig.height     = SCREEN_HEIGHT;
        windowConfig.title      = "Boat Example - ECS Rendering";
        windowConfig.vsync      = true;
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

    ~FishingGame()
    {
        // Cleanup audio
        m_gameEngine->getAudioSystem().shutdown();
    }

    /**
     * @brief Saves the current scene to a file
     * @param filepath Path to save the scene to
     */
    void saveScene(const std::string& filepath)
    {
        try
        {
            // Ensure the directory exists
            std::cout << "Saving scene to: " << filepath << std::endl;
            std::filesystem::path path(filepath);
            if (path.has_parent_path() && !std::filesystem::exists(path.parent_path()))
            {
                std::filesystem::create_directories(path.parent_path());
            }

            m_gameEngine->getSceneManager().saveScene(filepath);
            std::cout << "Scene saved to: " << filepath << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to save scene: " << e.what() << std::endl;
        }
    }

    /**
     * @brief Attempts to load a scene from a file
     * @param filepath Path to the scene file
     * @return true if scene was loaded successfully, false otherwise
     */
    bool loadScene(const std::string& filepath)
    {
        if (!std::filesystem::exists(filepath))
        {
            std::cout << "Scene file not found: " << filepath << std::endl;
            return false;
        }

        try
        {
            m_gameEngine->getSceneManager().loadScene(filepath);
            std::cout << "Scene loaded from: " << filepath << std::endl;

            // Process pending entities so they're available in entity map
            m_gameEngine->getEntityManager().update(0.0f);

            // Post-load: bind runtime resources that aren't serialized
            bindRuntimeResources();

            m_sceneLoaded = true;
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Failed to load scene: " << e.what() << std::endl;
            return false;
        }
    }

    /**
     * @brief Binds runtime resources after loading a scene
     *
     * This handles resources that truly cannot be serialized:
     * - Particle emitter textures (sf::Texture* pointers)
     * - Cached entity references (m_player, m_oceanBackground, etc.)
     * - Input controller callbacks (function pointers)
     *
     * Note: Physics bodies, colliders, textures, and shaders are initialized
     * automatically by the engine during deserialization.
     */
    void bindRuntimeResources()
    {
        auto& entityManager = m_gameEngine->getEntityManager();

        // Find and cache key entity references
        auto players = entityManager.getEntitiesByTag("player");
        if (!players.empty())
        {
            m_player        = players[0];
            m_playerPhysics = m_player->getComponent<CPhysicsBody2D>();

            // Re-bind input controller callbacks (not serialized)
            bindPlayerInputCallbacks();
        }

        auto oceans = entityManager.getEntitiesByTag("ocean");
        if (!oceans.empty())
        {
            m_oceanBackground = oceans[0];
        }

        // Find particle emitter entities and bind textures
        auto bubbleTrails = entityManager.getEntitiesByTag("bubble_trail");
        if (!bubbleTrails.empty())
        {
            m_bubbleTrailEntity = bubbleTrails[0];
            auto* emitter       = m_bubbleTrailEntity->getComponent<CParticleEmitter>();
            if (emitter)
            {
                // Load and bind bubble texture
                if (m_bubbleTexture.loadFromFile("assets/textures/bubble.png"))
                {
                    m_bubbleTexture.setSmooth(true);
                    emitter->setTexture(&m_bubbleTexture);
                }
            }
        }

        auto hullSprays = entityManager.getEntitiesByTag("hull_spray");
        if (!hullSprays.empty())
        {
            m_hullSprayEntity  = hullSprays[0];
            m_hullSprayEmitter = m_hullSprayEntity->getComponent<CParticleEmitter>();
            if (m_hullSprayEmitter)
            {
                // Load and bind spray texture
                if (m_sprayTexture.loadFromFile("assets/textures/bubble.png"))
                {
                    m_sprayTexture.setSmooth(true);
                    m_hullSprayEmitter->setTexture(&m_sprayTexture);
                }
            }
        }

        // Bind textures to barrel emitters
        auto barrels = entityManager.getEntitiesByTag("barrel");
        for (auto& barrel : barrels)
        {
            auto* emitter = barrel->getComponent<CParticleEmitter>();
            if (emitter)
            {
                emitter->setTexture(&m_sprayTexture);
            }
        }

        std::cout << "Runtime resources bound successfully" << std::endl;
    }

    /**
     * @brief Binds input controller callbacks to the player entity
     *
     * Input callbacks are function pointers that can't be serialized,
     * so they must be re-bound after loading a scene.
     */
    void bindPlayerInputCallbacks()
    {
        if (!m_player)
            return;

        auto* inputController = m_player->getComponent<CInputController>();
        if (!inputController)
            return;

        // Set callbacks for movement actions
        inputController->setActionCallback("MoveForward",
                                           [this](ActionState state)
                                           {
                                               if ((state == ActionState::Held || state == ActionState::Pressed)
                                                   && m_playerPhysics && m_playerPhysics->isInitialized())
                                               {
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
                                           [this, inputController](ActionState state)
                                           {
                                               if ((state == ActionState::Held || state == ActionState::Pressed)
                                                   && m_playerPhysics && m_playerPhysics->isInitialized())
                                               {
                                                   // Rudder: apply a lateral force at the stern to create torque instead of applying pure torque
                                                   b2Vec2 forward = m_playerPhysics->getForwardVector();
                                                   b2Vec2 right   = m_playerPhysics->getRightVector();
                                                   b2Vec2 vel     = m_playerPhysics->getLinearVelocity();

                                                   // Signed velocity in forward direction; absForwardVel will be used for magnitude
                                                   float forwardVelSigned = forward.x * vel.x + forward.y * vel.y; // signed forward velocity
                                                   float absForwardVel = std::fabs(forwardVelSigned); // only forward/back velocity matters

                                                                    // Only allow rudder to act if moving above a small threshold along the forward axis
                                                                    if (absForwardVel < MIN_SPEED_FOR_STEERING)
                                                       return;

                                                   // Debug print current speed used by the rudder calc
                                                   (void)0; // debug prints removed

                                                   // Stern location (meters) behind the center of mass
                                                   b2Vec2 stern = m_playerPhysics->getPosition() - forward * RUDDER_OFFSET_METERS;

                                                   // Determine the lateral direction based on travel direction
                                                   // When moving forward, A should steer left -> apply rightward lateral to stern
                                                   // When moving backward, steering is reversed
                                                   b2Vec2 lateral = (forwardVelSigned >= 0.0f) ? right : b2Vec2{-right.x, -right.y};

                                                   // Soft smoothing curve using effective speed above MIN to allow a small 'barely moving' effect at the minimum
                                                   float speedEffective = std::max(0.0f, absForwardVel - MIN_SPEED_FOR_STEERING);
                                                   float normalized = speedEffective / (speedEffective + RUDDER_SMOOTH_K);
                                                   float speedFactor = RUDDER_MIN_EFFECTIVE_SCALE + normalized * (1.0f - RUDDER_MIN_EFFECTIVE_SCALE);

                                                   // Optionally, small boost when actively pressing throttle for keyboard responsiveness
                                                   // if (inputController->isActionDown("MoveForward") || inputController->isActionDown("MoveBackward"))
                                                   //     speedFactor = std::max(speedFactor, 0.2f);

                                                   // Compute force magnitude and apply at stern
                                                   float forceMag = PLAYER_TURNING_FORCE * RUDDER_FORCE_MULTIPLIER * speedFactor;
                                                   b2Vec2 force{lateral.x * forceMag, lateral.y * forceMag};

                                                   m_playerPhysics->applyForce(force, stern);
                                               }
                                           });

        inputController->setActionCallback("RotateRight",
                                           [this, inputController](ActionState state)
                                           {
                                               if ((state == ActionState::Held || state == ActionState::Pressed)
                                                   && m_playerPhysics && m_playerPhysics->isInitialized())
                                               {
                                                   // Rudder: apply a lateral force at the stern to create torque instead of applying pure torque
                                                   b2Vec2 forward = m_playerPhysics->getForwardVector();
                                                   b2Vec2 right   = m_playerPhysics->getRightVector();
                                                   b2Vec2 vel     = m_playerPhysics->getLinearVelocity();

                                                   float forwardVelSigned = forward.x * vel.x + forward.y * vel.y; // signed forward velocity
                                                   float absForwardVel = std::fabs(forwardVelSigned); // only forward/back velocity matters

                                                                    // Only allow rudder to act if moving above a small threshold along the forward axis
                                                                    if (absForwardVel < MIN_SPEED_FOR_STEERING)
                                                       return;

                                                   // Debug print current speed used by the rudder calc
                                                   (void)0; // debug prints removed

                                                   // Stern location (meters) behind the center of mass
                                                   b2Vec2 stern = m_playerPhysics->getPosition() - forward * RUDDER_OFFSET_METERS;

                                                   // Determine lateral direction (invert when reversing)
                                                   b2Vec2 lateral = (forwardVelSigned >= 0.0f) ? b2Vec2{-right.x, -right.y} : right;

                                                   // Soft smoothing curve using effective speed above MIN to allow a small 'barely moving' effect at the minimum
                                                   float speedEffective = std::max(0.0f, absForwardVel - MIN_SPEED_FOR_STEERING);
                                                   float normalized = speedEffective / (speedEffective + RUDDER_SMOOTH_K);
                                                   float speedFactor = RUDDER_MIN_EFFECTIVE_SCALE + normalized * (1.0f - RUDDER_MIN_EFFECTIVE_SCALE);

                                                   // Compute force magnitude and apply at stern
                                                   float forceMag = PLAYER_TURNING_FORCE * RUDDER_FORCE_MULTIPLIER * speedFactor;
                                                   b2Vec2 force{lateral.x * forceMag, lateral.y * forceMag};

                                                   m_playerPhysics->applyForce(force, stern);
                                               }
                                           });
    }

    /**
     * @brief Creates the scene manually (used when no saved scene exists)
     * @note This is now a fallback - the game expects a saved scene file to exist.
     *       To create a new scene, temporarily uncomment the creation calls below,
     *       run the game, and press F5 to save.
     */
    void createSceneManually()
    {
        std::cerr << "ERROR: No saved scene found at " << getSceneFilePath() << std::endl;
        std::cerr << "Please ensure a valid scene file exists, or uncomment the creation code below." << std::endl;

        // Uncomment these lines to create a new scene from scratch:
        // createOceanBackground();
        // createBoundaryColliders();
        // createPlayer();
        // createBubbleTrail();
        // createHullSpray();
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

        // Initialize particle system
        auto& particleSystem = m_gameEngine->getParticleSystem();
        particleSystem.initialize(getWindow(), PIXELS_PER_METER);

        // Pre-load particle textures for effects
        if (m_sprayTexture.loadFromFile("assets/textures/bubble.png"))
        {
            m_sprayTexture.setSmooth(true);
            std::cout << "Loaded spray texture for emitters" << std::endl;
        }

        // Create a backup of the scene file before loading
        createSceneBackup(getSceneFilePath());

        // Try to load scene from file, otherwise create manually
        if (!loadScene(getSceneFilePath()))
        {
            std::cout << "Creating scene manually..." << std::endl;
            createSceneManually();
        }

        // Force EntityManager to process pending entities
        m_gameEngine->getEntityManager().update(0.0f);

        std::cout << "Game initialized!" << std::endl;
        std::cout << "Physics: Box2D v3.1.1 (1 unit = 1 meter, Y-up)" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD            : Move player boat (W=forward, S=backward, A/D=turn when moving forward)" << std::endl;
        std::cout << "  R               : Restart scenario" << std::endl;
        std::cout << "  G               : Toggle gravity" << std::endl;
        std::cout << "  C               : Toggle collider visibility" << std::endl;
        std::cout << "  V               : Toggle vector visualization" << std::endl;
        std::cout << "  F5              : Save scene" << std::endl;
        std::cout << "  F9              : Load scene" << std::endl;
        std::cout << "  Escape          : Exit" << std::endl;
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;
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

        if (m_showVectors)
        {
            createVelocityLines();
        }
        else
        {
            destroyVelocityLines();
        }
    }

    void createVelocityLines()
    {
        // Create velocity line entities for physics objects
        auto players = m_gameEngine->getEntityManager().getEntitiesByTag("player");

        for (auto& entity : players)
        {
            if (entity->hasComponent<CPhysicsBody2D>())
            {
                auto velocityLine = m_gameEngine->getEntityManager().addEntity("velocity_line");
                velocityLine->addComponent<CTransform>(Vec2(0, 0), Vec2(1.0f, 1.0f), 0.0f);
                auto* renderable = velocityLine->addComponent<CRenderable>(VisualType::Line, Color::Yellow, 1000, true);
                renderable->setLineStart(Vec2(0.0f, 0.0f));
                renderable->setLineEnd(Vec2(0.0f, 0.0f));
                renderable->setLineThickness(2.0f);

                m_velocityLines[entity.get()] = velocityLine;
            }
        }
    }

    void destroyVelocityLines()
    {
        for (auto& [entity, line] : m_velocityLines)
        {
            line->destroy();
        }
        m_velocityLines.clear();
    }

    void updateHullSpray()
    {
        if (!m_hullSprayEmitter || !m_playerPhysics || !m_playerPhysics->isInitialized())
            return;

        // Get current speed
        b2Vec2 velocity = m_playerPhysics->getLinearVelocity();
        float  speed    = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

        // Define speed thresholds
        const float MIN_SPEED_FOR_SPRAY = 0.1f;     // Start spraying at this speed (m/s)
        const float MAX_SPEED_FOR_SPRAY = 2.25f;    // Maximum spray at this speed (m/s)
        const float MIN_EMISSION_RATE   = 0.0f;     // Emission rate at minimum speed
        const float MAX_EMISSION_RATE   = 5000.0f;  // Emission rate at maximum speed

        // Calculate emission rate based on speed
        float emissionRate = 0.0f;
        if (speed > MIN_SPEED_FOR_SPRAY)
        {
            // Normalize speed to 0-1 range
            float normalizedSpeed = (speed - MIN_SPEED_FOR_SPRAY) / (MAX_SPEED_FOR_SPRAY - MIN_SPEED_FOR_SPRAY);
            normalizedSpeed       = std::min(1.0f, std::max(0.0f, normalizedSpeed));

            // Use quadratic curve for more dramatic effect at higher speeds
            emissionRate = MIN_EMISSION_RATE + (MAX_EMISSION_RATE - MIN_EMISSION_RATE) * (normalizedSpeed * normalizedSpeed);
        }

        m_hullSprayEmitter->setEmissionRate(emissionRate);

        // Also scale particle speed based on boat speed
        float speedMultiplier = 1.0f + (speed / MAX_SPEED_FOR_SPRAY) * 0.5f;
        m_hullSprayEmitter->setMinSpeed(0.1f * speedMultiplier);
        m_hullSprayEmitter->setMaxSpeed(0.4f * speedMultiplier);
    }

    void updateBarrelSprays()
    {
        // Get all barrel entities and update their spray emitters based on velocity
        auto barrels = m_gameEngine->getEntityManager().getEntitiesByTag("barrel");

        for (auto& barrel : barrels)
        {
            if (!barrel || !barrel->isAlive())
                continue;

            auto* physicsBody = barrel->getComponent<CPhysicsBody2D>();
            auto* emitter     = barrel->getComponent<CParticleEmitter>();

            if (!physicsBody || !physicsBody->isInitialized() || !emitter)
                continue;

            // Get current velocity
            b2Vec2 velocity = physicsBody->getLinearVelocity();
            float  speed    = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

            // Define speed thresholds for barrels
            const float MIN_SPEED_FOR_SPRAY = 0.05f;    // Start spraying at this speed (m/s)
            const float MAX_SPEED_FOR_SPRAY = 1.0f;     // Maximum spray at this speed (m/s)
            const float MIN_EMISSION_RATE   = 0.0f;     // Emission rate at minimum speed
            const float MAX_EMISSION_RATE   = 1250.0f;  // Emission rate at maximum speed

            // Calculate emission rate based on speed
            float emissionRate = 0.0f;
            if (speed > MIN_SPEED_FOR_SPRAY)
            {
                // Normalize speed to 0-1 range
                float normalizedSpeed = (speed - MIN_SPEED_FOR_SPRAY) / (MAX_SPEED_FOR_SPRAY - MIN_SPEED_FOR_SPRAY);
                normalizedSpeed       = std::min(1.0f, std::max(0.0f, normalizedSpeed));

                // Use quadratic curve for more dramatic effect at higher speeds
                emissionRate = MIN_EMISSION_RATE + (MAX_EMISSION_RATE - MIN_EMISSION_RATE) * (normalizedSpeed * normalizedSpeed);

                // Scale particle speed based on barrel speed for more realistic spray
                float speedMultiplier = 0.5f + (normalizedSpeed * 0.5f);
                emitter->setMinSpeed(0.15f * speedMultiplier);
                emitter->setMaxSpeed(0.5f * speedMultiplier);
            }

            emitter->setEmissionRate(emissionRate);
        }
    }

    void updateParticleEmitterPositions()
    {
        if (!m_player)
            return;

        auto* playerTransform = m_player->getComponent<CTransform>();
        if (!playerTransform)
            return;

        Vec2  playerPos      = playerTransform->getPosition();
        float playerRotation = playerTransform->getRotation();

        // Update bubble trail entity to follow player
        if (m_bubbleTrailEntity && m_bubbleTrailEntity->isAlive())
        {
            auto* trailTransform = m_bubbleTrailEntity->getComponent<CTransform>();
            if (trailTransform)
            {
                trailTransform->setPosition(playerPos);
                trailTransform->setRotation(playerRotation);
            }
        }

        // Update hull spray entity to follow player
        if (m_hullSprayEntity && m_hullSprayEntity->isAlive())
        {
            auto* sprayTransform = m_hullSprayEntity->getComponent<CTransform>();
            if (sprayTransform)
            {
                sprayTransform->setPosition(playerPos);
                sprayTransform->setRotation(playerRotation);
            }
        }
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

    void restart()
    {
        std::cout << "\n=== Restarting scenario ===" << std::endl;
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;

        // Stop motor boat if playing
        auto& audioSystem = m_gameEngine->getAudioSystem();
        if (audioSystem.isPlayingSFX(m_motorBoatHandle))
        {
            audioSystem.stopSFX(m_motorBoatHandle);
            m_motorBoatHandle = AudioHandle::invalid();
        }

        // Clear velocity lines
        m_velocityLines.clear();

        // Clear all entities
        m_gameEngine->getEntityManager().clear();

        // Reset physics world
        auto& physics = m_gameEngine->getPhysics();
        physics.setGravity({0.0f, m_gravityEnabled ? GRAVITY_FORCE : 0.0f});

        // Try to reload from scene file if one was previously loaded, otherwise create manually
        if (m_sceneLoaded && std::filesystem::exists(getSceneFilePath()))
        {
            if (!loadScene(getSceneFilePath()))
            {
                std::cout << "Failed to reload scene, creating manually..." << std::endl;
                createSceneManually();
            }
        }
        else
        {
            createSceneManually();
        }

        // Recreate velocity lines if vectors are visible
        if (m_showVectors)
        {
            createVelocityLines();
        }

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

    void updateVelocityLines()
    {
        const float VELOCITY_SCALE = 0.5f;  // Scale factor for velocity visualization

        // Update existing velocity lines
        auto allEntitiesToUpdate = m_velocityLines;
        for (auto& [entity, line] : allEntitiesToUpdate)
        {
            if (!entity || !entity->isAlive())
            {
                // Remove velocity line if entity is destroyed
                if (line)
                {
                    line->destroy();
                }
                m_velocityLines.erase(entity);
                continue;
            }

            auto* physicsBody = entity->getComponent<CPhysicsBody2D>();
            auto* transform   = entity->getComponent<CTransform>();
            auto* lineRender  = line->getComponent<CRenderable>();

            if (physicsBody && transform && lineRender)
            {
                // Get velocity
                b2Vec2 velocity = physicsBody->getLinearVelocity();
                Vec2   velocityVec(velocity.x, velocity.y);
                float  speed = velocityVec.length();

                if (speed > 0.01f)  // Only show if velocity is significant
                {
                    // Update line position to entity position
                    line->getComponent<CTransform>()->setPosition(transform->getPosition());

                    // Update line endpoints (local space)
                    lineRender->setLineStart(Vec2(0.0f, 0.0f));
                    lineRender->setLineEnd(velocityVec * VELOCITY_SCALE);
                    lineRender->setVisible(true);
                }
                else
                {
                    lineRender->setVisible(false);
                }
            }
        }

        // Check for new entities that need velocity lines
        auto players = m_gameEngine->getEntityManager().getEntitiesByTag("player");

        for (auto& entity : players)
        {
            if (entity->hasComponent<CPhysicsBody2D>() && m_velocityLines.find(entity.get()) == m_velocityLines.end())
            {
                // Create new velocity line for this entity
                auto velocityLine = m_gameEngine->getEntityManager().addEntity("velocity_line");
                velocityLine->addComponent<CTransform>(Vec2(0, 0), Vec2(1.0f, 1.0f), 0.0f);
                auto* renderable = velocityLine->addComponent<CRenderable>(VisualType::Line, Color::Yellow, 1000, true);
                renderable->setLineStart(Vec2(0.0f, 0.0f));
                renderable->setLineEnd(Vec2(0.0f, 0.0f));
                renderable->setLineThickness(2.0f);

                m_velocityLines[entity.get()] = velocityLine;
            }
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
        if (im.wasKeyPressed(KeyCode::F5))
        {
            std::cout << "Saving scene to " << getSceneFilePath() << std::endl;
            saveScene(getSceneFilePath());
        }
        if (im.wasKeyPressed(KeyCode::F9))
        {
            // Clear and reload scene
            std::cout << "Reloading scene from " << getSceneFilePath() << std::endl;
            m_velocityLines.clear();
            m_gameEngine->getEntityManager().clear();
            if (loadScene(getSceneFilePath()))
            {
                if (m_showVectors)
                {
                    createVelocityLines();
                }
                m_gameEngine->getEntityManager().update(0.0f);
            }
            else
            {
                std::cout << "No saved scene to load, creating manually..." << std::endl;
                createSceneManually();
                m_gameEngine->getEntityManager().update(0.0f);
            }
        }

        // Update Box2D physics
        m_gameEngine->getPhysics().update(dt);

        // Update particle emitter positions to follow the player
        updateParticleEmitterPositions();

        // Update hull spray emission rate based on boat speed
        updateHullSpray();

        // Update barrel spray emission rates based on their speeds
        updateBarrelSprays();

        // Update particle system
        m_gameEngine->getParticleSystem().update(dt);

        // Update ocean shader uniforms with object positions
        // updateOceanShaderUniforms();

        // Update velocity lines if visible
        if (m_showVectors)
        {
            updateVelocityLines();
        }

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

        // Draw UI text showing current status
        if (m_fontLoaded)
        {
            const auto& audioSystem   = m_gameEngine->getAudioSystem();
            float       currentVolume = audioSystem.getMasterVolume();

            std::ostringstream oss;
            oss << "Box2D Physics (1 unit = 1 meter, Y-up) - ECS Rendering Pipeline\n";
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
