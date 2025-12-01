#include "components/CParticleEmitter.h"
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
    m_maxParticles      = static_cast<int>(emitter["maxParticles"].getNumber());
}

size_t CParticleEmitter::getAliveCount() const
{
    size_t count = 0;
    for (const auto& p : m_particles)
    {
        if (p.alive)
            count++;
    }
    return count;
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

// Texture management
sf::Texture* CParticleEmitter::getTexture() const
{
    return m_texture;
}

void CParticleEmitter::setTexture(sf::Texture* tex)
{
    m_texture = tex;
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
