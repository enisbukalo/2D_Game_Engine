#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <SFML/Graphics.hpp>

/**
 * @brief Main game engine class handling core game loop and systems
 *
 * @description
 * GameEngine is the central class that manages the game loop, physics updates,
 * and rendering. It provides a fixed timestep update system for consistent
 * physics simulation and handles input processing. The engine integrates with
 * SFML for window management and rendering.
 */
class GameEngine
{
public:
#pragma region Constructors
    /**
     * @brief Constructs a game engine instance
     * @param window Pointer to the SFML render window
     * @param gravity The global gravity vector
     * @param subStepCount Number of physics sub-steps per update
     * @param timeStep Fixed time step for physics updates
     */
    GameEngine(sf::RenderWindow* window, sf::Vector2f gravity, uint8_t subStepCount, float timeStep);

    /** @brief Destructor */
    ~GameEngine();
#pragma endregion

#pragma region Methods
    /**
     * @brief Processes input events
     */
    void readInputs();

    /**
     * @brief Updates game logic and physics
     */
    void update();

    /**
     * @brief Renders the current game state
     */
    void render();

    /**
     * @brief Checks if the game is still running
     * @return true if the game is running, false otherwise
     */
    bool is_running() const;
#pragma endregion

private:
#pragma region Variables
    const uint8_t m_subStepCount;  ///< Number of physics sub-steps per update
    const float   m_timeStep;      ///< Fixed time step for physics updates

    bool m_gameRunning = false;  ///< Flag indicating if the game is running

    sf::RenderWindow* m_window;   ///< Pointer to the SFML render window
    sf::Vector2f      m_gravity;  ///< Global gravity vector
#pragma endregion
};

#endif  // GAMEENGINE_H