#ifndef SPARTICLESYSTEM_H
#define SPARTICLESYSTEM_H

#include <SFML/Graphics.hpp>
#include "System.h"

// Forward declarations
class EntityManager;

/**
 * @brief Particle system that updates and renders particles from CParticleEmitter components
 *
 * @description
 * SParticleSystem is responsible for updating particle physics and rendering
 * particles for all entities that have a CParticleEmitter component. It follows
 * the ECS pattern where the system operates on component data.
 *
 * Features:
 * - Updates particle physics (position, velocity, lifetime)
 * - Renders particles efficiently using SFML vertex arrays
 * - Supports textured and colored particles
 * - Automatic particle lifecycle management
 */
class SParticleSystem : public System
{
public:
    /**
     * @brief Gets the singleton instance
     * @return Reference to the singleton instance
     */
    static SParticleSystem& instance();

    /**
     * @brief Initializes the particle system
     * @param window SFML render window (used to get dimensions)
     * @param pixelsPerMeter Rendering scale (pixels per meter)
     * @return true if initialization succeeded
     */
    bool initialize(sf::RenderWindow* window, float pixelsPerMeter = 100.0f);

    /**
     * @brief Shuts down the particle system
     */
    void shutdown();

    /**
     * @brief Updates all particle emitters on entities
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Renders all particles to a render window
     * @param window SFML render window
     */
    void render(sf::RenderWindow* window);

    /**
     * @brief Checks if the particle system is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const
    {
        return m_initialized;
    }

private:
    /** @brief Private constructor for singleton pattern */
    SParticleSystem();

    /** @brief Destructor */
    ~SParticleSystem() override;

    /** @brief Deleted copy constructor */
    SParticleSystem(const SParticleSystem&) = delete;

    /** @brief Deleted assignment operator */
    SParticleSystem& operator=(const SParticleSystem&) = delete;

    /**
     * @brief Converts world coordinates (meters) to screen coordinates (pixels)
     * @param worldPos Position in meters
     * @return Position in pixels
     */
    sf::Vector2f worldToScreen(const Vec2& worldPos) const;

    /**
     * @brief Converts meters to pixels (for size)
     * @param meters Size in meters
     * @return Size in pixels
     */
    float metersToPixels(float meters) const;

    sf::VertexArray   m_vertexArray;     ///< Vertex array for rendering
    sf::RenderWindow* m_window;          ///< Render window reference
    float             m_pixelsPerMeter;  ///< Rendering scale
    bool              m_initialized;     ///< Initialization state
};

#endif  // SPARTICLESYSTEM_H
