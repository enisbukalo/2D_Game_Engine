#include "components/CParticleEmitter.h"
#include <algorithm>
#include "JsonBuilder.h"
#include "JsonValue.h"

// Particle constructor
Particle::Particle()
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

// CParticleEmitter constructor
CParticleEmitter::CParticleEmitter() = default;

std::string CParticleEmitter::getType() const
{
    return "CParticleEmitter";
}

void CParticleEmitter::serialize(JsonBuilder& builder) const
{
    builder.beginObject();
    builder.addKey("cParticleEmitter");
    builder.beginObject();
    builder.addKey("offset");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_positionOffset.x);
    builder.addKey("y");
    builder.addNumber(m_positionOffset.y);
    builder.endObject();
    builder.addKey("active");
    builder.addBool(Component::isActive());
    builder.addKey("maxParticles");
    builder.addNumber(m_maxParticles);
    builder.endObject();
    builder.endObject();
}

void CParticleEmitter::deserialize(const JsonValue& value)
{
    const auto& emitter = value["cParticleEmitter"];
    const auto& offset  = emitter["offset"];
    m_positionOffset.x  = offset["x"].getNumber();
    m_positionOffset.y  = offset["y"].getNumber();
    Component::setActive(emitter["active"].getBool());
    m_maxParticles = static_cast<int>(emitter["maxParticles"].getNumber());
}

size_t CParticleEmitter::getAliveCount() const
{
    return std::count_if(m_particles.begin(), m_particles.end(), [](const Particle& p) { return p.alive; });
}

// Configuration getters/setters
Vec2 CParticleEmitter::getDirection() const
{
    return m_direction;
}

void CParticleEmitter::setDirection(const Vec2& dir)
{
    m_direction = dir;
}

float CParticleEmitter::getSpreadAngle() const
{
    return m_spreadAngle;
}

void CParticleEmitter::setSpreadAngle(float angle)
{
    m_spreadAngle = angle;
}

float CParticleEmitter::getMinSpeed() const
{
    return m_minSpeed;
}

void CParticleEmitter::setMinSpeed(float speed)
{
    m_minSpeed = speed;
}

float CParticleEmitter::getMaxSpeed() const
{
    return m_maxSpeed;
}

void CParticleEmitter::setMaxSpeed(float speed)
{
    m_maxSpeed = speed;
}

float CParticleEmitter::getMinLifetime() const
{
    return m_minLifetime;
}

void CParticleEmitter::setMinLifetime(float lifetime)
{
    m_minLifetime = lifetime;
}

float CParticleEmitter::getMaxLifetime() const
{
    return m_maxLifetime;
}

void CParticleEmitter::setMaxLifetime(float lifetime)
{
    m_maxLifetime = lifetime;
}

float CParticleEmitter::getMinSize() const
{
    return m_minSize;
}

void CParticleEmitter::setMinSize(float size)
{
    m_minSize = size;
}

float CParticleEmitter::getMaxSize() const
{
    return m_maxSize;
}

void CParticleEmitter::setMaxSize(float size)
{
    m_maxSize = size;
}

float CParticleEmitter::getEmissionRate() const
{
    return m_emissionRate;
}

void CParticleEmitter::setEmissionRate(float rate)
{
    m_emissionRate = rate;
}

float CParticleEmitter::getBurstCount() const
{
    return m_burstCount;
}

void CParticleEmitter::setBurstCount(float count)
{
    m_burstCount = count;
}

Color CParticleEmitter::getStartColor() const
{
    return m_startColor;
}

void CParticleEmitter::setStartColor(const Color& color)
{
    m_startColor = color;
}

Color CParticleEmitter::getEndColor() const
{
    return m_endColor;
}

void CParticleEmitter::setEndColor(const Color& color)
{
    m_endColor = color;
}

float CParticleEmitter::getStartAlpha() const
{
    return m_startAlpha;
}

void CParticleEmitter::setStartAlpha(float alpha)
{
    m_startAlpha = alpha;
}

float CParticleEmitter::getEndAlpha() const
{
    return m_endAlpha;
}

void CParticleEmitter::setEndAlpha(float alpha)
{
    m_endAlpha = alpha;
}

Vec2 CParticleEmitter::getGravity() const
{
    return m_gravity;
}

void CParticleEmitter::setGravity(const Vec2& grav)
{
    m_gravity = grav;
}

float CParticleEmitter::getMinRotationSpeed() const
{
    return m_minRotationSpeed;
}

void CParticleEmitter::setMinRotationSpeed(float speed)
{
    m_minRotationSpeed = speed;
}

float CParticleEmitter::getMaxRotationSpeed() const
{
    return m_maxRotationSpeed;
}

void CParticleEmitter::setMaxRotationSpeed(float speed)
{
    m_maxRotationSpeed = speed;
}

bool CParticleEmitter::getFadeOut() const
{
    return m_fadeOut;
}

void CParticleEmitter::setFadeOut(bool fade)
{
    m_fadeOut = fade;
}

bool CParticleEmitter::getShrink() const
{
    return m_shrink;
}

void CParticleEmitter::setShrink(bool shrinkEnabled)
{
    m_shrink = shrinkEnabled;
}

float CParticleEmitter::getShrinkEndScale() const
{
    return m_shrinkEndScale;
}

void CParticleEmitter::setShrinkEndScale(float scale)
{
    m_shrinkEndScale = scale;
}

int CParticleEmitter::getMaxParticles() const
{
    return m_maxParticles;
}

void CParticleEmitter::setMaxParticles(int max)
{
    m_maxParticles = max;
}

Vec2 CParticleEmitter::getPositionOffset() const
{
    return m_positionOffset;
}

void CParticleEmitter::setPositionOffset(const Vec2& offset)
{
    m_positionOffset = offset;
}

// Emission shape configuration
EmissionShape CParticleEmitter::getEmissionShape() const
{
    return m_emissionShape;
}

void CParticleEmitter::setEmissionShape(EmissionShape shape)
{
    m_emissionShape = shape;
}

float CParticleEmitter::getShapeRadius() const
{
    return m_shapeRadius;
}

void CParticleEmitter::setShapeRadius(float radius)
{
    m_shapeRadius = radius;
}

Vec2 CParticleEmitter::getShapeSize() const
{
    return m_shapeSize;
}

void CParticleEmitter::setShapeSize(const Vec2& size)
{
    m_shapeSize = size;
}

Vec2 CParticleEmitter::getLineStart() const
{
    return m_lineStart;
}

void CParticleEmitter::setLineStart(const Vec2& start)
{
    m_lineStart = start;
}

Vec2 CParticleEmitter::getLineEnd() const
{
    return m_lineEnd;
}

void CParticleEmitter::setLineEnd(const Vec2& end)
{
    m_lineEnd = end;
}

bool CParticleEmitter::getEmitFromEdge() const
{
    return m_emitFromEdge;
}

void CParticleEmitter::setEmitFromEdge(bool edge)
{
    m_emitFromEdge = edge;
}

bool CParticleEmitter::getEmitOutward() const
{
    return m_emitOutward;
}

void CParticleEmitter::setEmitOutward(bool outward)
{
    m_emitOutward = outward;
}

const std::vector<Vec2>& CParticleEmitter::getPolygonVertices() const
{
    return m_polygonVertices;
}

void CParticleEmitter::setPolygonVertices(const std::vector<Vec2>& vertices)
{
    m_polygonVertices = vertices;
}

void CParticleEmitter::addPolygonVertex(const Vec2& vertex)
{
    m_polygonVertices.push_back(vertex);
}

void CParticleEmitter::clearPolygonVertices()
{
    m_polygonVertices.clear();
}

void CParticleEmitter::setPolygonFromConvexHull(const std::vector<Vec2>& vertices)
{
    m_polygonVertices.clear();

    if (vertices.size() < 3)
    {
        // Not enough vertices for a polygon
        m_polygonVertices = vertices;
        return;
    }

    // Make a copy to work with
    std::vector<Vec2> points = vertices;

    // Find the bottom-most point (lowest Y, then lowest X as tiebreaker)
    size_t startIdx = 0;
    for (size_t i = 1; i < points.size(); ++i)
    {
        if (points[i].y < points[startIdx].y || (points[i].y == points[startIdx].y && points[i].x < points[startIdx].x))
        {
            startIdx = i;
        }
    }
    std::swap(points[0], points[startIdx]);
    Vec2 pivot = points[0];

    // Sort by polar angle relative to pivot (counter-clockwise)
    std::sort(points.begin() + 1,
              points.end(),
              [&pivot](const Vec2& a, const Vec2& b)
              {
                  Vec2  da           = a - pivot;
                  Vec2  db           = b - pivot;
                  float crossProduct = da.x * db.y - da.y * db.x;
                  if (std::abs(crossProduct) < 1e-9f)
                  {
                      // Collinear - sort by distance (closer first)
                      return da.x * da.x + da.y * da.y < db.x * db.x + db.y * db.y;
                  }
                  return crossProduct > 0;  // Counter-clockwise order
              });

    // Build convex hull using Graham scan
    std::vector<Vec2> hull;
    for (const auto& v : points)
    {
        // Remove points that make a clockwise turn (or are collinear)
        while (hull.size() >= 2)
        {
            Vec2  a     = hull[hull.size() - 2];
            Vec2  b     = hull[hull.size() - 1];
            Vec2  ab    = b - a;
            Vec2  ac    = v - a;
            float cross = ab.x * ac.y - ab.y * ac.x;
            if (cross <= 0)
                hull.pop_back();
            else
                break;
        }
        hull.push_back(v);
    }

    m_polygonVertices = hull;
}

// Texture management
sf::Texture* CParticleEmitter::getTexture() const
{
    return m_texture;
}

void CParticleEmitter::setTexture(sf::Texture* tex)
{
    m_texture = tex;
}

// Z-index for render ordering
int CParticleEmitter::getZIndex() const
{
    return m_zIndex;
}

void CParticleEmitter::setZIndex(int zIndex)
{
    m_zIndex = zIndex;
}

// Runtime state access
std::vector<Particle>& CParticleEmitter::getParticles()
{
    return m_particles;
}

const std::vector<Particle>& CParticleEmitter::getParticles() const
{
    return m_particles;
}

float CParticleEmitter::getEmissionTimer() const
{
    return m_emissionTimer;
}

void CParticleEmitter::setEmissionTimer(float timer)
{
    m_emissionTimer = timer;
}
