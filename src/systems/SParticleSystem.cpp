#include "SParticleSystem.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include "CParticleEmitter.h"
#include "CTransform.h"
#include "EntityManager.h"

// Static random number generator
static std::random_device               s_rd;
static std::mt19937                     s_gen(s_rd());
static std::uniform_real_distribution<> s_dist(0.0, 1.0);

// Vector to store entity ids with no emitters
static std::vector<size_t> s_entitiesWithoutEmitters;

// Helper function to generate random float
static float randomFloat(float min, float max)
{
    return min + static_cast<float>(s_dist(s_gen)) * (max - min);
}

// Helper function to interpolate between two values
template <typename T>
static T lerp(const T& a, const T& b, float t)
{
    return a + (b - a) * t;
}

// Helper function to interpolate between two colors
static Color lerpColor(const Color& a, const Color& b, float t)
{
    return Color(static_cast<unsigned char>(lerp(static_cast<float>(a.r), static_cast<float>(b.r), t)),
                 static_cast<unsigned char>(lerp(static_cast<float>(a.g), static_cast<float>(b.g), t)),
                 static_cast<unsigned char>(lerp(static_cast<float>(a.b), static_cast<float>(b.b), t)),
                 static_cast<unsigned char>(lerp(static_cast<float>(a.a), static_cast<float>(b.a), t)));
}

//=============================================================================
// Particle emission and update logic
//=============================================================================

static Particle spawnParticle(CParticleEmitter* emitter, const Vec2& worldPosition)
{
    Particle p;
    p.alive = true;
    p.age   = 0.0f;

    // Position
    p.position = worldPosition;

    // Lifetime
    p.lifetime = randomFloat(emitter->minLifetime, emitter->maxLifetime);

    // Size
    p.size        = randomFloat(emitter->minSize, emitter->maxSize);
    p.initialSize = p.size;

    // Velocity (direction + spread + speed)
    float angle  = std::atan2(emitter->direction.y, emitter->direction.x);
    float spread = randomFloat(-emitter->spreadAngle, emitter->spreadAngle);
    angle += spread;

    float speed = randomFloat(emitter->minSpeed, emitter->maxSpeed);
    p.velocity  = Vec2(std::cos(angle) * speed, std::sin(angle) * speed);

    // Acceleration (gravity)
    p.acceleration = emitter->gravity;

    // Color and alpha
    p.color = emitter->startColor;
    p.alpha = emitter->startAlpha;

    // Rotation
    p.rotation      = randomFloat(0.0f, 2.0f * 3.14159f);
    p.rotationSpeed = randomFloat(emitter->minRotationSpeed, emitter->maxRotationSpeed);

    return p;
}

static void updateParticle(Particle& particle, CParticleEmitter* emitter, float deltaTime)
{
    particle.age += deltaTime;

    // Kill if lifetime exceeded
    if (particle.age >= particle.lifetime)
    {
        particle.alive = false;
        return;
    }

    // Update physics
    particle.velocity.x += particle.acceleration.x * deltaTime;
    particle.velocity.y += particle.acceleration.y * deltaTime;
    particle.position.x += particle.velocity.x * deltaTime;
    particle.position.y += particle.velocity.y * deltaTime;

    // Update rotation
    particle.rotation += particle.rotationSpeed * deltaTime;

    // Interpolation factor (0 at birth, 1 at death)
    float t = particle.age / particle.lifetime;

    // Color interpolation
    particle.color = lerpColor(emitter->startColor, emitter->endColor, t);

    // Alpha fade
    if (emitter->fadeOut)
    {
        particle.alpha = lerp(emitter->startAlpha, emitter->endAlpha, t);
    }

    // Size shrink
    if (emitter->shrink)
    {
        particle.size = particle.initialSize * lerp(1.0f, emitter->shrinkEndScale, t);
    }
}

static void emitParticle(CParticleEmitter* emitter, const Vec2& worldPosition)
{
    // Check particle limit
    if (emitter->getAliveCount() >= static_cast<size_t>(emitter->maxParticles))
    {
        return;
    }

    // Find dead particle to reuse or add new one
    for (auto& p : emitter->particles)
    {
        if (p.alive == false)
        {
            p = spawnParticle(emitter, worldPosition);
            return;
        }
    }

    // No dead particles, add new one
    emitter->particles.push_back(spawnParticle(emitter, worldPosition));
}

//=============================================================================
// SParticleSystem Implementation
//=============================================================================

SParticleSystem& SParticleSystem::instance()
{
    static SParticleSystem instance;
    return instance;
}

SParticleSystem::SParticleSystem()
    : m_vertexArray(sf::Quads), m_window(nullptr), m_pixelsPerMeter(100.0f), m_initialized(false)
{
}

SParticleSystem::~SParticleSystem()
{
    shutdown();
}

bool SParticleSystem::initialize(sf::RenderWindow* window, float pixelsPerMeter)
{
    m_window         = window;
    m_pixelsPerMeter = pixelsPerMeter;
    m_initialized    = true;
    return true;
}

void SParticleSystem::shutdown()
{
    m_initialized = false;
    m_window      = nullptr;
}

void SParticleSystem::update(float deltaTime)
{
    if (m_initialized == false)
    {
        std::cout << "Particle system not initialized. Call initialize() first." << std::endl;
        return;
    }

    // Iterate over all entities with CParticleEmitter component
    auto entities = EntityManager::instance().getEntities();

    for (auto& entity : entities)
    {
        bool hasEmitter   = entity->hasComponent<CParticleEmitter>();
        bool hasTransform = entity->hasComponent<CTransform>();

        if (hasEmitter == false || hasTransform == false)
        {
            continue;
        }

        auto* emitter   = entity->getComponent<CParticleEmitter>();
        auto* transform = entity->getComponent<CTransform>();

        if (emitter->active == false)
        {
            std::cout << "Emitter on entity " << entity->getId() << " is inactive." << std::endl;
            continue;
        }

        // Calculate world position (entity position + rotated offset)
        Vec2 entityPos = transform->getPosition();
        Vec2 offset    = emitter->positionOffset;

        float rotation = transform->getRotation();
        if (rotation != 0.0f)
        {
            float cosR     = std::cos(rotation);
            float sinR     = std::sin(rotation);
            float rotatedX = offset.x * cosR - offset.y * sinR;
            float rotatedY = offset.x * sinR + offset.y * cosR;
            offset         = Vec2(rotatedX, rotatedY);
        }

        Vec2 worldPos = entityPos + offset;

        // Update existing particles
        for (auto& particle : emitter->particles)
        {
            if (particle.alive)
            {
                updateParticle(particle, emitter, deltaTime);
            }
        }

        // Emit new particles if needed
        if (emitter->emissionRate > 0.0f)
        {
            emitter->emissionTimer += deltaTime;
            float emissionInterval = 1.0f / emitter->emissionRate;

            while (emitter->emissionTimer >= emissionInterval)
            {
                emitParticle(emitter, worldPos);
                emitter->emissionTimer -= emissionInterval;
            }
        }
    }
}

void SParticleSystem::render(sf::RenderWindow* window)
{
    // Use stored window if no window parameter provided
    sf::RenderWindow* targetWindow = window ? window : m_window;

    if (m_initialized == false || targetWindow == nullptr)
    {
        std::cout << "Particle system not initialized or no render window available." << std::endl;
        return;
    }

    // Iterate over all entities with CParticleEmitter component
    auto entities = EntityManager::instance().getEntities();

    for (auto& entity : entities)
    {
        if (entity->hasComponent<CParticleEmitter>() == false)
        {
            // if entity id has NOT been stored yet, print and store
            if (std::find(s_entitiesWithoutEmitters.begin(), s_entitiesWithoutEmitters.end(), entity->getId())
                == s_entitiesWithoutEmitters.end())
            {
                s_entitiesWithoutEmitters.push_back(entity->getId());
                std::cout << "Entity " << entity->getId() << " has no CParticleEmitter component." << std::endl;
                std::cout << "Entity Tag: " << entity->getTag() << std::endl;
            }
            continue;
        }

        auto* emitter = entity->getComponent<CParticleEmitter>();

        // Clear vertex array for this emitter
        m_vertexArray.clear();

        // Build vertex array for all alive particles
        for (const auto& particle : emitter->particles)
        {
            if (particle.alive == false)
            {
                continue;
            }

            // Convert to screen space
            sf::Vector2f screenPos = worldToScreen(particle.position);
            float        pixelSize = metersToPixels(particle.size);

            // Create color with alpha
            sf::Color color(particle.color.r, particle.color.g, particle.color.b, static_cast<sf::Uint8>(particle.alpha * 255.0f));

            if (emitter->texture)
            {
                // Textured quad
                float cosR = std::cos(particle.rotation);
                float sinR = std::sin(particle.rotation);

                // Quad corners (centered)
                sf::Vector2f corners[4] = {
                    sf::Vector2f(-pixelSize, -pixelSize),  // Top-left
                    sf::Vector2f(pixelSize, -pixelSize),   // Top-right
                    sf::Vector2f(pixelSize, pixelSize),    // Bottom-right
                    sf::Vector2f(-pixelSize, pixelSize)    // Bottom-left
                };

                // Rotate corners
                for (int i = 0; i < 4; ++i)
                {
                    float x      = corners[i].x;
                    float y      = corners[i].y;
                    corners[i].x = x * cosR - y * sinR;
                    corners[i].y = x * sinR + y * cosR;
                    corners[i] += screenPos;
                }

                // Texture coordinates (in pixels for SFML - confirmed by official docs)
                sf::Vector2u texSize      = emitter->texture->getSize();
                sf::Vector2f texCoords[4] = {
                    sf::Vector2f(0.0f, 0.0f),                                                    // Top-left
                    sf::Vector2f(static_cast<float>(texSize.x), 0.0f),                           // Top-right
                    sf::Vector2f(static_cast<float>(texSize.x), static_cast<float>(texSize.y)),  // Bottom-right
                    sf::Vector2f(0.0f, static_cast<float>(texSize.y))                            // Bottom-left
                };

                // Add vertices
                for (int i = 0; i < 4; ++i)
                {
                    m_vertexArray.append(sf::Vertex(corners[i], color, texCoords[i]));
                }
            }
            else
            {
                // Colored quad (no texture)
                float cosR = std::cos(particle.rotation);
                float sinR = std::sin(particle.rotation);

                sf::Vector2f corners[4] = {sf::Vector2f(-pixelSize, -pixelSize),
                                           sf::Vector2f(pixelSize, -pixelSize),
                                           sf::Vector2f(pixelSize, pixelSize),
                                           sf::Vector2f(-pixelSize, pixelSize)};

                for (int i = 0; i < 4; ++i)
                {
                    float x      = corners[i].x;
                    float y      = corners[i].y;
                    corners[i].x = x * cosR - y * sinR;
                    corners[i].y = x * sinR + y * cosR;
                    corners[i] += screenPos;
                }

                for (int i = 0; i < 4; ++i)
                {
                    m_vertexArray.append(sf::Vertex(corners[i], color));
                }
            }
        }

        // Render particles for this emitter
        if (m_vertexArray.getVertexCount() > 0)
        {
            // Debug: Print first particle position and size
            if (emitter->particles.size() > 0 && emitter->particles[0].alive)
            {
                const auto&  p         = emitter->particles[0];
                sf::Vector2f screenPos = worldToScreen(p.position);
                float        pixelSize = metersToPixels(p.size);
                std::cout << "First particle - World: (" << p.position.x << ", " << p.position.y << ") Screen: ("
                          << screenPos.x << ", " << screenPos.y << ") Size: " << pixelSize << "px" << std::endl;
            }

            std::cout << "Rendering " << (m_vertexArray.getVertexCount() / 4) << " particles for entity "
                      << entity->getId() << " (texture: " << (emitter->texture ? "YES" : "NO") << ", texSize: "
                      << (emitter->texture ? std::to_string(emitter->texture->getSize().x) + "x"
                                                 + std::to_string(emitter->texture->getSize().y)
                                           : "N/A")
                      << ")" << std::endl;

            sf::RenderStates states;
            states.blendMode = sf::BlendAlpha;

            if (emitter->texture)
            {
                states.texture = emitter->texture;
            }

            targetWindow->draw(m_vertexArray, states);
        }
    }
}

sf::Vector2f SParticleSystem::worldToScreen(const Vec2& worldPos) const
{
    float screenX      = worldPos.x * m_pixelsPerMeter;
    float screenHeight = m_window ? m_window->getSize().y : 600.0f;
    float screenY      = screenHeight - (worldPos.y * m_pixelsPerMeter);
    return sf::Vector2f(screenX, screenY);
}

float SParticleSystem::metersToPixels(float meters) const
{
    return meters * m_pixelsPerMeter;
}
