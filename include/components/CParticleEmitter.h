#ifndef CPARTICLEEMITTER_H
#define CPARTICLEEMITTER_H

#include "Component.h"
#include "Color.h"
#include "Vec2.h"
#include "JsonBuilder.h"
#include "JsonValue.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

/**
 * @brief Individual particle data structure
 */
struct Particle
{
    Vec2  position;         ///< Current position in world space (meters)
    Vec2  velocity;         ///< Current velocity (meters/second)
    Vec2  acceleration;     ///< Current acceleration (meters/second²)
    Color color;            ///< Particle color
    float alpha;            ///< Alpha transparency (0.0 to 1.0)
    float lifetime;         ///< Total lifetime in seconds
    float age;              ///< Current age in seconds
    float size;             ///< Particle size (radius in meters)
    float initialSize;      ///< Initial size for shrink effect
    float rotation;         ///< Current rotation in radians
    float rotationSpeed;    ///< Rotation speed in radians/second
    bool  alive;            ///< Is particle still alive?

    Particle()
        : position(0, 0),
          velocity(0, 0),
          acceleration(0, 0),
          color(Color::White),
          alpha(1.0f),
          lifetime(1.0f),
          age(0.0f),
          size(0.5f),
          initialSize(0.5f),
          rotation(0.0f),
          rotationSpeed(0.0f),
          alive(false)
    {
    }
};

/**
 * @brief Component that defines a particle emitter attached to an entity
 * 
 * This component contains all particle emission configuration and state.
 * The emitter automatically follows the entity's transform position.
 */
class CParticleEmitter : public Component
{
  public:
    CParticleEmitter() = default;
    ~CParticleEmitter() override = default;

    std::string getType() const override { return "CParticleEmitter"; }

    // Configuration
    Vec2  direction         = Vec2(0, 1);     ///< Base emission direction (normalized)
    float spreadAngle       = 0.5f;           ///< Spread angle in radians (cone of emission)
    float minSpeed          = 0.1f;           ///< Minimum initial speed (m/s)
    float maxSpeed          = 0.3f;           ///< Maximum initial speed (m/s)
    float minLifetime       = 1.0f;           ///< Minimum particle lifetime (seconds)
    float maxLifetime       = 1.5f;           ///< Maximum particle lifetime (seconds)
    float minSize           = 0.50f;          ///< Minimum particle size (meters)
    float maxSize           = 0.50f;          ///< Maximum particle size (meters)
    float emissionRate      = 10.0f;          ///< Particles per second
    float burstCount        = 0.0f;           ///< Number of particles to emit in a burst
    Color startColor        = Color::White;   ///< Starting color
    Color endColor          = Color::Cyan;    ///< Ending color (for color interpolation)
    float startAlpha        = 1.0f;           ///< Starting alpha
    float endAlpha          = 1.0f;           ///< Ending alpha
    Vec2  gravity           = Vec2(0, 0.2f);  ///< Gravity/constant acceleration (m/s²)
    float minRotationSpeed  = -1.0f;          ///< Minimum rotation speed (radians/second)
    float maxRotationSpeed  = 1.0f;           ///< Maximum rotation speed (radians/second)
    bool  fadeOut           = true;           ///< Should particles fade out over lifetime?
    bool  shrink            = true;           ///< Should particles shrink over lifetime?
    float shrinkEndScale    = 0.1f;           ///< Final size scale when fully shrunk
    bool  active            = true;           ///< Is emitter currently active?
    int   maxParticles      = 200;            ///< Maximum number of particles
    sf::Texture* texture    = nullptr;        ///< Optional texture for particles

    // Offset from entity position (in local space, affected by rotation)
    Vec2 positionOffset = Vec2(0.0f, 0.0f);

    // Runtime state
    std::vector<Particle> particles;          ///< All particles (alive and dead)
    float emissionTimer = 0.0f;               ///< Time accumulator for continuous emission

    /**
     * @brief Get number of alive particles
     */
    size_t getAliveCount() const
    {
        size_t count = 0;
        for (const auto& p : particles)
        {
            if (p.alive) count++;
        }
        return count;
    }

    /**
     * @brief Serialize component to JSON
     */
    void serialize(JsonBuilder& builder) const override
    {
        builder.beginObject();
        builder.addKey("cParticleEmitter");
        builder.beginObject();
        builder.addKey("offset");
        builder.beginObject();
        builder.addKey("x");
        builder.addNumber(positionOffset.x);
        builder.addKey("y");
        builder.addNumber(positionOffset.y);
        builder.endObject();
        builder.addKey("active");
        builder.addBool(active);
        builder.addKey("maxParticles");
        builder.addNumber(maxParticles);
        builder.endObject();
        builder.endObject();
    }

    /**
     * @brief Deserialize component from JSON
     */
    void deserialize(const JsonValue& value) override
    {
        const auto& emitter = value["cParticleEmitter"];
        const auto& offset  = emitter["offset"];
        positionOffset.x    = offset["x"].getNumber();
        positionOffset.y    = offset["y"].getNumber();
        active              = emitter["active"].getBool();
        maxParticles        = static_cast<int>(emitter["maxParticles"].getNumber());
    }
};

#endif  // CPARTICLEEMITTER_H
