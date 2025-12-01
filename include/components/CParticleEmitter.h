#ifndef CPARTICLEEMITTER_H
#define CPARTICLEEMITTER_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Color.h"
#include "Component.h"
#include "Vec2.h"

/**
 * @brief Individual particle data structure
 */
struct Particle
{
    Vec2  position;       ///< Current position in world space (meters)
    Vec2  velocity;       ///< Current velocity (meters/second)
    Vec2  acceleration;   ///< Current acceleration (meters/second²)
    Color color;          ///< Particle color
    float alpha;          ///< Alpha transparency (0.0 to 1.0)
    float lifetime;       ///< Total lifetime in seconds
    float age;            ///< Current age in seconds
    float size;           ///< Particle size (radius in meters)
    float initialSize;    ///< Initial size for shrink effect
    float rotation;       ///< Current rotation in radians
    float rotationSpeed;  ///< Rotation speed in radians/second
    bool  alive;          ///< Is particle still alive?

    Particle();
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
    CParticleEmitter();
    ~CParticleEmitter() override = default;

    /**
     * @brief Gets the type identifier for this component
     * @return String "CParticleEmitter"
     */
    std::string getType() const override;

    /**
     * @brief Serialize component to JSON
     */
    void serialize(JsonBuilder& builder) const override;

    /**
     * @brief Deserialize component from JSON
     */
    void deserialize(const JsonValue& value) override;

    /**
     * @brief Get number of alive particles
     * @return Count of currently alive particles
     */
    size_t getAliveCount() const;

    // Configuration getters/setters
    Vec2  getDirection() const;
    void  setDirection(const Vec2& dir);
    float getSpreadAngle() const;
    void  setSpreadAngle(float angle);
    float getMinSpeed() const;
    void  setMinSpeed(float speed);
    float getMaxSpeed() const;
    void  setMaxSpeed(float speed);
    float getMinLifetime() const;
    void  setMinLifetime(float lifetime);
    float getMaxLifetime() const;
    void  setMaxLifetime(float lifetime);
    float getMinSize() const;
    void  setMinSize(float size);
    float getMaxSize() const;
    void  setMaxSize(float size);
    float getEmissionRate() const;
    void  setEmissionRate(float rate);
    float getBurstCount() const;
    void  setBurstCount(float count);
    Color getStartColor() const;
    void  setStartColor(const Color& color);
    Color getEndColor() const;
    void  setEndColor(const Color& color);
    float getStartAlpha() const;
    void  setStartAlpha(float alpha);
    float getEndAlpha() const;
    void  setEndAlpha(float alpha);
    Vec2  getGravity() const;
    void  setGravity(const Vec2& grav);
    float getMinRotationSpeed() const;
    void  setMinRotationSpeed(float speed);
    float getMaxRotationSpeed() const;
    void  setMaxRotationSpeed(float speed);
    bool  getFadeOut() const;
    void  setFadeOut(bool fade);
    bool  getShrink() const;
    void  setShrink(bool shrinkEnabled);
    float getShrinkEndScale() const;
    void  setShrinkEndScale(float scale);
    int   getMaxParticles() const;
    void  setMaxParticles(int max);
    Vec2  getPositionOffset() const;
    void  setPositionOffset(const Vec2& offset);

    // Texture management
    sf::Texture* getTexture() const;
    void         setTexture(sf::Texture* tex);

    // Runtime state access
    std::vector<Particle>&       getParticles();
    const std::vector<Particle>& getParticles() const;
    float                        getEmissionTimer() const;
    void                         setEmissionTimer(float timer);

private:
    // Configuration
    Vec2  m_direction        = Vec2(0, 1);        ///< Base emission direction (normalized)
    float m_spreadAngle      = 0.5f;              ///< Spread angle in radians (cone of emission)
    float m_minSpeed         = 0.1f;              ///< Minimum initial speed (m/s)
    float m_maxSpeed         = 0.3f;              ///< Maximum initial speed (m/s)
    float m_minLifetime      = 1.0f;              ///< Minimum particle lifetime (seconds)
    float m_maxLifetime      = 1.5f;              ///< Maximum particle lifetime (seconds)
    float m_minSize          = 0.50f;             ///< Minimum particle size (meters)
    float m_maxSize          = 0.50f;             ///< Maximum particle size (meters)
    float m_emissionRate     = 10.0f;             ///< Particles per second
    float m_burstCount       = 0.0f;              ///< Number of particles to emit in a burst
    Color m_startColor       = Color::White;      ///< Starting color
    Color m_endColor         = Color::Cyan;       ///< Ending color (for color interpolation)
    float m_startAlpha       = 1.0f;              ///< Starting alpha
    float m_endAlpha         = 1.0f;              ///< Ending alpha
    Vec2  m_gravity          = Vec2(0, 0.2f);     ///< Gravity/constant acceleration (m/s²)
    float m_minRotationSpeed = -1.0f;             ///< Minimum rotation speed (radians/second)
    float m_maxRotationSpeed = 1.0f;              ///< Maximum rotation speed (radians/second)
    bool  m_fadeOut          = true;              ///< Should particles fade out over lifetime?
    bool  m_shrink           = true;              ///< Should particles shrink over lifetime?
    float m_shrinkEndScale   = 0.1f;              ///< Final size scale when fully shrunk
    int   m_maxParticles     = 200;               ///< Maximum number of particles
    Vec2  m_positionOffset   = Vec2(0.0f, 0.0f);  ///< Offset from entity position

    // Resources
    sf::Texture* m_texture = nullptr;  ///< Optional texture for particles

    // Runtime state
    std::vector<Particle> m_particles;             ///< All particles (alive and dead)
    float                 m_emissionTimer = 0.0f;  ///< Time accumulator for continuous emission
};

#endif  // CPARTICLEEMITTER_H
