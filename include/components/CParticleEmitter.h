#ifndef CPARTICLEEMITTER_H
#define CPARTICLEEMITTER_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Color.h"
#include "Component.h"
#include "Vec2.h"

/**
 * @brief Emission shape type for particle emitters
 *
 * Defines how particles are distributed when spawned:
 * - Point: All particles spawn from a single point (default)
 * - Circle: Particles spawn on the edge of a circle
 * - Rectangle: Particles spawn on the edges of a rectangle
 * - Line: Particles spawn along a line segment
 * - Polygon: Particles spawn on the edges of a custom polygon
 */
enum class EmissionShape
{
    Point,      ///< Single point emission (default)
    Circle,     ///< Emit from circle edge
    Rectangle,  ///< Emit from rectangle edges
    Line,       ///< Emit along a line segment
    Polygon     ///< Emit from polygon edges
};

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

    // Emission shape configuration
    EmissionShape getEmissionShape() const;
    void          setEmissionShape(EmissionShape shape);
    float         getShapeRadius() const;
    void          setShapeRadius(float radius);
    Vec2          getShapeSize() const;
    void          setShapeSize(const Vec2& size);
    Vec2          getLineStart() const;
    void          setLineStart(const Vec2& start);
    Vec2          getLineEnd() const;
    void          setLineEnd(const Vec2& end);
    bool          getEmitFromEdge() const;
    void          setEmitFromEdge(bool edge);
    bool          getEmitOutward() const;
    void          setEmitOutward(bool outward);

    // Polygon shape configuration
    const std::vector<Vec2>& getPolygonVertices() const;
    void                     setPolygonVertices(const std::vector<Vec2>& vertices);
    void                     addPolygonVertex(const Vec2& vertex);
    void                     clearPolygonVertices();

    /**
     * @brief Set polygon vertices from a collection of points, computing convex hull
     *
     * Takes a set of vertices (e.g., from multiple collider fixtures) and computes
     * the convex hull to get the outer boundary. Useful for extracting emission
     * shape from complex multi-fixture colliders.
     *
     * @param vertices All vertices to compute hull from
     */
    void setPolygonFromConvexHull(const std::vector<Vec2>& vertices);

    // Texture management
    sf::Texture* getTexture() const;
    void         setTexture(sf::Texture* tex);

    // Z-index for render ordering
    int  getZIndex() const;
    void setZIndex(int zIndex);

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
    int   m_zIndex           = 0;                 ///< Render layer (lower = behind)
    Vec2  m_positionOffset   = Vec2(0.0f, 0.0f);  ///< Offset from entity position

    // Emission shape configuration
    EmissionShape     m_emissionShape = EmissionShape::Point;  ///< Shape for emission distribution
    float             m_shapeRadius   = 1.0f;                  ///< Radius for circle shape (meters)
    Vec2              m_shapeSize     = Vec2(1.0f, 1.0f);      ///< Size for rectangle shape (meters)
    Vec2              m_lineStart     = Vec2(-0.5f, 0.0f);     ///< Start point for line shape
    Vec2              m_lineEnd       = Vec2(0.5f, 0.0f);      ///< End point for line shape
    std::vector<Vec2> m_polygonVertices;                       ///< Vertices for polygon shape
    bool              m_emitFromEdge = true;                   ///< Emit from edge (true) or filled area (false)
    bool              m_emitOutward  = false;                  ///< Emit in direction away from shape center

    // Resources
    sf::Texture* m_texture = nullptr;  ///< Optional texture for particles

    // Runtime state
    std::vector<Particle> m_particles;             ///< All particles (alive and dead)
    float                 m_emissionTimer = 0.0f;  ///< Time accumulator for continuous emission
};

#endif  // CPARTICLEEMITTER_H
