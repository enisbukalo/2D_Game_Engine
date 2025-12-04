#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <spdlog/spdlog.h>
#include <memory>

// Include system and manager headers
#include <ComponentFactory.h>
#include <EntityManager.h>
#include <SAudioSystem.h>
#include <SBox2DPhysics.h>
#include <SInputManager.h>
#include <SParticleSystem.h>
#include <SRenderer.h>
#include <SceneManager.h>

/**
 * @brief Main game engine class handling core game loop and systems
 *
 * @description
 * GameEngine is the central class that manages the game loop, physics updates,
 * and rendering. It provides a fixed timestep update system for consistent
 * physics simulation and handles input processing. The engine uses SRenderer
 * for window management and rendering, abstracting away direct SFML dependencies.
 */
class GameEngine
{
public:
    /**
     * @brief Constructs a game engine instance
     * @param windowConfig Window initialization configuration
     * @param gravity The global gravity vector (Y-up: positive Y = upward)
     * @param subStepCount Number of physics sub-steps per update (default: 6, increase for more stability with many bodies)
     * @param timeStep Fixed time step for physics updates
     */
    GameEngine(const WindowConfig& windowConfig, Vec2 gravity, uint8_t subStepCount = 6, float timeStep = 1.0f / 60.0f);

    /** @brief Destructor */
    ~GameEngine();

    /**
     * @brief Processes input events
     */
    void readInputs();

    /**
     * @brief Updates game logic and physics
     * @param deltaTime Time elapsed since last update in seconds (variable timestep)
     *
     * Physics updates use fixed timestep internally for stability, while other
     * systems can use the variable deltaTime if needed.
     */
    void update(float deltaTime);

    /**
     * @brief Renders the current game state
     */
    void render();

    /**
     * @brief Checks if the game is still running
     * @return true if the game is running, false otherwise
     */
    bool is_running() const;

    /**
     * @brief Gets the logger instance for external logging
     * @return Shared pointer to the spdlog logger, or nullptr if not initialized
     */
    static std::shared_ptr<spdlog::logger> getLogger();

    // System and Manager Accessors
    // These are the recommended public API for engine users

    /**
     * @brief Gets the entity manager instance
     * @return Reference to the EntityManager singleton
     */
    EntityManager& getEntityManager();

    /**
     * @brief Gets the scene manager instance
     * @return Reference to the SceneManager singleton
     */
    SceneManager& getSceneManager();

    /**
     * @brief Gets the component factory instance
     * @return Reference to the ComponentFactory singleton
     */
    ComponentFactory& getComponentFactory();

    /**
     * @brief Gets the Box2D physics system instance
     * @return Reference to the SBox2DPhysics singleton
     */
    SBox2DPhysics& getPhysics();

    /**
     * @brief Gets the input manager instance
     * @return Reference to the SInputManager singleton
     */
    SInputManager& getInputManager();

    /**
     * @brief Gets the audio system instance
     * @return Reference to the SAudioSystem singleton
     */
    SAudioSystem& getAudioSystem();

    /**
     * @brief Gets the renderer system instance
     * @return Reference to the SRenderer singleton
     */
    SRenderer& getRenderer();

    /**
     * @brief Gets the particle system instance
     * @return Reference to the SParticleSystem singleton
     */
    SParticleSystem& getParticleSystem();

private:
    const uint8_t m_subStepCount;  ///< Number of physics sub-steps per update
    const float   m_timeStep;      ///< Fixed time step for physics updates

    bool  m_gameRunning = false;  ///< Flag indicating if the game is running
    float m_accumulator = 0.0f;   ///< Accumulator for fixed timestep updates

    Vec2 m_gravity;  ///< Global gravity vector
};

#endif  // GAMEENGINE_H