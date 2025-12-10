#ifndef SPARTICLE_H
#define SPARTICLE_H

#include <Entity.h>
#include <SFML/Graphics.hpp>
#include "System.h"

class Registry;  // Forward declaration

namespace Systems
{

/**
 * @brief Particle system that updates and renders particles from CParticleEmitter components
 *
 * @description
 * SParticle is responsible for updating particle physics and rendering
 * particles for all entities that have a CParticleEmitter component. It follows
 * the ECS pattern where the system operates on component data.
 *
 * Features:
 * - Updates particle physics (position, velocity, lifetime)
 * - Renders particles efficiently using SFML vertex arrays
 * - Supports textured and colored particles
 * - Automatic particle lifecycle management
 */
class SParticle : public System
{
public:
    /**
     * @brief Gets the singleton instance
     * @return Reference to the singleton instance
     */
    static SParticle& instance();

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
     * @brief Renders particles for a single emitter entity
     * @param entity Entity ID with CParticleEmitter component
     * @param window SFML render window
     * @param registry Registry to access components
     */
    void renderEmitter(Entity entity, sf::RenderWindow* window, Registry& registry);

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
    SParticle();

    /** @brief Destructor */
    ~SParticle() override;

    /** @brief Deleted copy constructor */
    SParticle(const SParticle&) = delete;

    /** @brief Deleted assignment operator */
    SParticle& operator=(const SParticle&) = delete;

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

}  // namespace Systems

#endif  // SPARTICLESYSTEM_H
