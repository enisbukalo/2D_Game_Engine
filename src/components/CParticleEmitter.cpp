#include "components/CParticleEmitter.h"
#include <algorithm>
#include "systems/SSerialization.h"

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

    // Component GUID
    builder.addKey("guid");
    builder.addString(getGuid());

    // Base component properties
    builder.addKey("active");
    builder.addBool(Component::isActive());

    // Emission direction and spread
    builder.addKey("direction");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_direction.x);
    builder.addKey("y");
    builder.addNumber(m_direction.y);
    builder.endObject();
    builder.addKey("spreadAngle");
    builder.addNumber(m_spreadAngle);

    // Speed range
    builder.addKey("minSpeed");
    builder.addNumber(m_minSpeed);
    builder.addKey("maxSpeed");
    builder.addNumber(m_maxSpeed);

    // Lifetime range
    builder.addKey("minLifetime");
    builder.addNumber(m_minLifetime);
    builder.addKey("maxLifetime");
    builder.addNumber(m_maxLifetime);

    // Size range
    builder.addKey("minSize");
    builder.addNumber(m_minSize);
    builder.addKey("maxSize");
    builder.addNumber(m_maxSize);

    // Emission rate and burst
    builder.addKey("emissionRate");
    builder.addNumber(m_emissionRate);
    builder.addKey("burstCount");
    builder.addNumber(m_burstCount);

    // Colors
    builder.addKey("startColor");
    builder.beginObject();
    builder.addKey("r");
    builder.addNumber(m_startColor.r);
    builder.addKey("g");
    builder.addNumber(m_startColor.g);
    builder.addKey("b");
    builder.addNumber(m_startColor.b);
    builder.addKey("a");
    builder.addNumber(m_startColor.a);
    builder.endObject();

    builder.addKey("endColor");
    builder.beginObject();
    builder.addKey("r");
    builder.addNumber(m_endColor.r);
    builder.addKey("g");
    builder.addNumber(m_endColor.g);
    builder.addKey("b");
    builder.addNumber(m_endColor.b);
    builder.addKey("a");
    builder.addNumber(m_endColor.a);
    builder.endObject();

    // Alpha range
    builder.addKey("startAlpha");
    builder.addNumber(m_startAlpha);
    builder.addKey("endAlpha");
    builder.addNumber(m_endAlpha);

    // Gravity
    builder.addKey("gravity");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_gravity.x);
    builder.addKey("y");
    builder.addNumber(m_gravity.y);
    builder.endObject();

    // Rotation speed range
    builder.addKey("minRotationSpeed");
    builder.addNumber(m_minRotationSpeed);
    builder.addKey("maxRotationSpeed");
    builder.addNumber(m_maxRotationSpeed);

    // Visual effects
    builder.addKey("fadeOut");
    builder.addBool(m_fadeOut);
    builder.addKey("shrink");
    builder.addBool(m_shrink);
    builder.addKey("shrinkEndScale");
    builder.addNumber(m_shrinkEndScale);

    // Particle limits
    builder.addKey("maxParticles");
    builder.addNumber(m_maxParticles);
    builder.addKey("zIndex");
    builder.addNumber(m_zIndex);

    // Position offset
    builder.addKey("offset");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_positionOffset.x);
    builder.addKey("y");
    builder.addNumber(m_positionOffset.y);
    builder.endObject();

    // Emission shape configuration
    builder.addKey("emissionShape");
    builder.addNumber(static_cast<int>(m_emissionShape));

    builder.addKey("shapeRadius");
    builder.addNumber(m_shapeRadius);

    builder.addKey("shapeSize");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_shapeSize.x);
    builder.addKey("y");
    builder.addNumber(m_shapeSize.y);
    builder.endObject();

    builder.addKey("lineStart");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_lineStart.x);
    builder.addKey("y");
    builder.addNumber(m_lineStart.y);
    builder.endObject();

    builder.addKey("lineEnd");
    builder.beginObject();
    builder.addKey("x");
    builder.addNumber(m_lineEnd.x);
    builder.addKey("y");
    builder.addNumber(m_lineEnd.y);
    builder.endObject();

    builder.addKey("emitFromEdge");
    builder.addBool(m_emitFromEdge);
    builder.addKey("emitOutward");
    builder.addBool(m_emitOutward);

    // Polygon vertices
    builder.addKey("polygonVertices");
    builder.beginArray();
    for (const auto& vertex : m_polygonVertices)
    {
        builder.beginObject();
        builder.addKey("x");
        builder.addNumber(vertex.x);
        builder.addKey("y");
        builder.addNumber(vertex.y);
        builder.endObject();
    }
    builder.endArray();

    builder.endObject();
    builder.endObject();
}

void CParticleEmitter::deserialize(const JsonValue& value)
{
    const auto& emitter = value["cParticleEmitter"];

    // Component GUID
    if (emitter.hasKey("guid"))
    {
        setGuid(emitter["guid"].getString());
    }

    // Base component properties
    if (emitter["active"].isBool())
    {
        Component::setActive(emitter["active"].getBool());
    }

    // Emission direction and spread
    const auto& direction = emitter["direction"];
    if (direction.isObject())
    {
        m_direction.x = static_cast<float>(direction["x"].getNumber());
        m_direction.y = static_cast<float>(direction["y"].getNumber());
    }
    if (emitter["spreadAngle"].isNumber())
    {
        m_spreadAngle = static_cast<float>(emitter["spreadAngle"].getNumber());
    }

    // Speed range
    if (emitter["minSpeed"].isNumber())
    {
        m_minSpeed = static_cast<float>(emitter["minSpeed"].getNumber());
    }
    if (emitter["maxSpeed"].isNumber())
    {
        m_maxSpeed = static_cast<float>(emitter["maxSpeed"].getNumber());
    }

    // Lifetime range
    if (emitter["minLifetime"].isNumber())
    {
        m_minLifetime = static_cast<float>(emitter["minLifetime"].getNumber());
    }
    if (emitter["maxLifetime"].isNumber())
    {
        m_maxLifetime = static_cast<float>(emitter["maxLifetime"].getNumber());
    }

    // Size range
    if (emitter["minSize"].isNumber())
    {
        m_minSize = static_cast<float>(emitter["minSize"].getNumber());
    }
    if (emitter["maxSize"].isNumber())
    {
        m_maxSize = static_cast<float>(emitter["maxSize"].getNumber());
    }

    // Emission rate and burst
    if (emitter["emissionRate"].isNumber())
    {
        m_emissionRate = static_cast<float>(emitter["emissionRate"].getNumber());
    }
    if (emitter["burstCount"].isNumber())
    {
        m_burstCount = static_cast<float>(emitter["burstCount"].getNumber());
    }

    // Colors
    const auto& startColor = emitter["startColor"];
    if (startColor.isObject())
    {
        m_startColor.r = static_cast<uint8_t>(startColor["r"].getNumber());
        m_startColor.g = static_cast<uint8_t>(startColor["g"].getNumber());
        m_startColor.b = static_cast<uint8_t>(startColor["b"].getNumber());
        m_startColor.a = static_cast<uint8_t>(startColor["a"].getNumber());
    }

    const auto& endColor = emitter["endColor"];
    if (endColor.isObject())
    {
        m_endColor.r = static_cast<uint8_t>(endColor["r"].getNumber());
        m_endColor.g = static_cast<uint8_t>(endColor["g"].getNumber());
        m_endColor.b = static_cast<uint8_t>(endColor["b"].getNumber());
        m_endColor.a = static_cast<uint8_t>(endColor["a"].getNumber());
    }

    // Alpha range
    if (emitter["startAlpha"].isNumber())
    {
        m_startAlpha = static_cast<float>(emitter["startAlpha"].getNumber());
    }
    if (emitter["endAlpha"].isNumber())
    {
        m_endAlpha = static_cast<float>(emitter["endAlpha"].getNumber());
    }

    // Gravity
    const auto& gravity = emitter["gravity"];
    if (gravity.isObject())
    {
        m_gravity.x = static_cast<float>(gravity["x"].getNumber());
        m_gravity.y = static_cast<float>(gravity["y"].getNumber());
    }

    // Rotation speed range
    if (emitter["minRotationSpeed"].isNumber())
    {
        m_minRotationSpeed = static_cast<float>(emitter["minRotationSpeed"].getNumber());
    }
    if (emitter["maxRotationSpeed"].isNumber())
    {
        m_maxRotationSpeed = static_cast<float>(emitter["maxRotationSpeed"].getNumber());
    }

    // Visual effects
    if (emitter["fadeOut"].isBool())
    {
        m_fadeOut = emitter["fadeOut"].getBool();
    }
    if (emitter["shrink"].isBool())
    {
        m_shrink = emitter["shrink"].getBool();
    }
    if (emitter["shrinkEndScale"].isNumber())
    {
        m_shrinkEndScale = static_cast<float>(emitter["shrinkEndScale"].getNumber());
    }

    // Particle limits
    if (emitter["maxParticles"].isNumber())
    {
        m_maxParticles = static_cast<int>(emitter["maxParticles"].getNumber());
    }
    if (emitter["zIndex"].isNumber())
    {
        m_zIndex = static_cast<int>(emitter["zIndex"].getNumber());
    }

    // Position offset (support both "offset" key for backward compat)
    const auto& offset = emitter["offset"];
    if (offset.isObject())
    {
        m_positionOffset.x = static_cast<float>(offset["x"].getNumber());
        m_positionOffset.y = static_cast<float>(offset["y"].getNumber());
    }

    // Emission shape configuration
    if (emitter["emissionShape"].isNumber())
    {
        m_emissionShape = static_cast<EmissionShape>(static_cast<int>(emitter["emissionShape"].getNumber()));
    }

    if (emitter["shapeRadius"].isNumber())
    {
        m_shapeRadius = static_cast<float>(emitter["shapeRadius"].getNumber());
    }

    const auto& shapeSize = emitter["shapeSize"];
    if (shapeSize.isObject())
    {
        m_shapeSize.x = static_cast<float>(shapeSize["x"].getNumber());
        m_shapeSize.y = static_cast<float>(shapeSize["y"].getNumber());
    }

    const auto& lineStart = emitter["lineStart"];
    if (lineStart.isObject())
    {
        m_lineStart.x = static_cast<float>(lineStart["x"].getNumber());
        m_lineStart.y = static_cast<float>(lineStart["y"].getNumber());
    }

    const auto& lineEnd = emitter["lineEnd"];
    if (lineEnd.isObject())
    {
        m_lineEnd.x = static_cast<float>(lineEnd["x"].getNumber());
        m_lineEnd.y = static_cast<float>(lineEnd["y"].getNumber());
    }

    if (emitter["emitFromEdge"].isBool())
    {
        m_emitFromEdge = emitter["emitFromEdge"].getBool();
    }
    if (emitter["emitOutward"].isBool())
    {
        m_emitOutward = emitter["emitOutward"].getBool();
    }

    // Polygon vertices
    const auto& polygonVertices = emitter["polygonVertices"];
    if (polygonVertices.isArray())
    {
        m_polygonVertices.clear();
        const auto& verticesArray = polygonVertices.getArray();
        for (size_t i = 0; i < verticesArray.size(); ++i)
        {
            const auto& vertex = verticesArray[i];
            if (vertex.isObject())
            {
                Vec2 v;
                v.x = static_cast<float>(vertex["x"].getNumber());
                v.y = static_cast<float>(vertex["y"].getNumber());
                m_polygonVertices.push_back(v);
            }
        }
    }
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
