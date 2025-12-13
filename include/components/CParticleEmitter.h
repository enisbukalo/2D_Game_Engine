#ifndef CPARTICLEEMITTER_H
#define CPARTICLEEMITTER_H

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>
#include "Color.h"
#include "Vec2.h"

namespace Components
{

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
    Vec2  position{0.0f, 0.0f};      ///< Current position in world space (meters)
    Vec2  velocity{0.0f, 0.0f};      ///< Current velocity (meters/second)
    Vec2  acceleration{0.0f, 0.0f};  ///< Current acceleration (meters/second²)
    Color color{Color::White};       ///< Particle color
    float alpha{1.0f};               ///< Alpha transparency (0.0 to 1.0)
    float lifetime{1.0f};            ///< Total lifetime in seconds
    float age{0.0f};                 ///< Current age in seconds
    float size{0.5f};                ///< Particle size (radius in meters)
    float initialSize{0.5f};         ///< Initial size for shrink effect
    float rotation{0.0f};            ///< Current rotation in radians
    float rotationSpeed{0.0f};       ///< Rotation speed in radians/second
    bool  alive{false};              ///< Is particle still alive?
};

/**
 * @brief Component that defines a particle emitter attached to an entity
 *
 * This component contains all particle emission configuration and state.
 * The emitter automatically follows the entity's transform position.
 */
struct CParticleEmitter
{
public:
    CParticleEmitter()  = default;
    ~CParticleEmitter() = default;

    /**
     * @brief Get number of alive particles
     * @return Count of currently alive particles
     */
    inline size_t getAliveCount() const
    {
        return static_cast<size_t>(
            std::count_if(m_particles.begin(), m_particles.end(), [](const Particle& p) { return p.alive; }));
    }

    bool isActive() const
    {
        return m_enabled;
    }
    void setActive(bool active)
    {
        m_enabled = active;
    }

    // Configuration getters/setters
    inline Vec2 getDirection() const
    {
        return m_direction;
    }
    inline void setDirection(const Vec2& dir)
    {
        m_direction = dir;
    }
    inline float getSpreadAngle() const
    {
        return m_spreadAngle;
    }
    inline void setSpreadAngle(float angle)
    {
        m_spreadAngle = angle;
    }
    inline float getMinSpeed() const
    {
        return m_minSpeed;
    }
    inline void setMinSpeed(float speed)
    {
        m_minSpeed = speed;
    }
    inline float getMaxSpeed() const
    {
        return m_maxSpeed;
    }
    inline void setMaxSpeed(float speed)
    {
        m_maxSpeed = speed;
    }
    inline float getMinLifetime() const
    {
        return m_minLifetime;
    }
    inline void setMinLifetime(float lifetime)
    {
        m_minLifetime = lifetime;
    }
    inline float getMaxLifetime() const
    {
        return m_maxLifetime;
    }
    inline void setMaxLifetime(float lifetime)
    {
        m_maxLifetime = lifetime;
    }
    inline float getMinSize() const
    {
        return m_minSize;
    }
    inline void setMinSize(float size)
    {
        m_minSize = size;
    }
    inline float getMaxSize() const
    {
        return m_maxSize;
    }
    inline void setMaxSize(float size)
    {
        m_maxSize = size;
    }
    inline float getEmissionRate() const
    {
        return m_emissionRate;
    }
    inline void setEmissionRate(float rate)
    {
        m_emissionRate = rate;
    }
    inline float getBurstCount() const
    {
        return m_burstCount;
    }
    inline void setBurstCount(float count)
    {
        m_burstCount = count;
    }
    inline Color getStartColor() const
    {
        return m_startColor;
    }
    inline void setStartColor(const Color& color)
    {
        m_startColor = color;
    }
    inline Color getEndColor() const
    {
        return m_endColor;
    }
    inline void setEndColor(const Color& color)
    {
        m_endColor = color;
    }
    inline float getStartAlpha() const
    {
        return m_startAlpha;
    }
    inline void setStartAlpha(float alpha)
    {
        m_startAlpha = alpha;
    }
    inline float getEndAlpha() const
    {
        return m_endAlpha;
    }
    inline void setEndAlpha(float alpha)
    {
        m_endAlpha = alpha;
    }
    inline Vec2 getGravity() const
    {
        return m_gravity;
    }
    inline void setGravity(const Vec2& grav)
    {
        m_gravity = grav;
    }
    inline float getMinRotationSpeed() const
    {
        return m_minRotationSpeed;
    }
    inline void setMinRotationSpeed(float speed)
    {
        m_minRotationSpeed = speed;
    }
    inline float getMaxRotationSpeed() const
    {
        return m_maxRotationSpeed;
    }
    inline void setMaxRotationSpeed(float speed)
    {
        m_maxRotationSpeed = speed;
    }
    inline bool getFadeOut() const
    {
        return m_fadeOut;
    }
    inline void setFadeOut(bool fade)
    {
        m_fadeOut = fade;
    }
    inline bool getShrink() const
    {
        return m_shrink;
    }
    inline void setShrink(bool shrinkEnabled)
    {
        m_shrink = shrinkEnabled;
    }
    inline float getShrinkEndScale() const
    {
        return m_shrinkEndScale;
    }
    inline void setShrinkEndScale(float scale)
    {
        m_shrinkEndScale = scale;
    }
    inline int getMaxParticles() const
    {
        return m_maxParticles;
    }
    inline void setMaxParticles(int max)
    {
        m_maxParticles = max;
    }
    inline Vec2 getPositionOffset() const
    {
        return m_positionOffset;
    }
    inline void setPositionOffset(const Vec2& offset)
    {
        m_positionOffset = offset;
    }

    // Emission shape configuration
    inline EmissionShape getEmissionShape() const
    {
        return m_emissionShape;
    }
    inline void setEmissionShape(EmissionShape shape)
    {
        m_emissionShape = shape;
    }
    inline float getShapeRadius() const
    {
        return m_shapeRadius;
    }
    inline void setShapeRadius(float radius)
    {
        m_shapeRadius = radius;
    }
    inline Vec2 getShapeSize() const
    {
        return m_shapeSize;
    }
    inline void setShapeSize(const Vec2& size)
    {
        m_shapeSize = size;
    }
    inline Vec2 getLineStart() const
    {
        return m_lineStart;
    }
    inline void setLineStart(const Vec2& start)
    {
        m_lineStart = start;
    }
    inline Vec2 getLineEnd() const
    {
        return m_lineEnd;
    }
    inline void setLineEnd(const Vec2& end)
    {
        m_lineEnd = end;
    }
    inline bool getEmitFromEdge() const
    {
        return m_emitFromEdge;
    }
    inline void setEmitFromEdge(bool edge)
    {
        m_emitFromEdge = edge;
    }
    inline bool getEmitOutward() const
    {
        return m_emitOutward;
    }
    inline void setEmitOutward(bool outward)
    {
        m_emitOutward = outward;
    }

    // Polygon shape configuration
    inline const std::vector<Vec2>& getPolygonVertices() const
    {
        return m_polygonVertices;
    }
    inline void setPolygonVertices(const std::vector<Vec2>& vertices)
    {
        m_polygonVertices = vertices;
    }
    inline void addPolygonVertex(const Vec2& vertex)
    {
        m_polygonVertices.push_back(vertex);
    }
    inline void clearPolygonVertices()
    {
        m_polygonVertices.clear();
    }

    // Texture configuration (resource ownership stays in systems)
    inline const std::string& getTexturePath() const
    {
        return m_texturePath;
    }
    inline void setTexturePath(const std::string& path)
    {
        m_texturePath = path;
    }

    // Z-index for render ordering
    inline int getZIndex() const
    {
        return m_zIndex;
    }
    inline void setZIndex(int zIndex)
    {
        m_zIndex = zIndex;
    }

    // Runtime state access
    inline std::vector<Particle>& getParticles()
    {
        return m_particles;
    }
    inline const std::vector<Particle>& getParticles() const
    {
        return m_particles;
    }
    inline float getEmissionTimer() const
    {
        return m_emissionTimer;
    }
    inline void setEmissionTimer(float timer)
    {
        m_emissionTimer = timer;
    }

private:
    bool m_enabled = true;  ///< Whether the emitter is active

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

    // Resources (by reference)
    std::string m_texturePath;  ///< Optional texture path for particles

    // Runtime state
    std::vector<Particle> m_particles;             ///< All particles (alive and dead)
    float                 m_emissionTimer = 0.0f;  ///< Time accumulator for continuous emission
};

}  // namespace Components

#endif  // CPARTICLEEMITTER_H
