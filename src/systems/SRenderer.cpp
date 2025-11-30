#include "SRenderer.h"
#include "CMaterial.h"
#include "CRenderable.h"
#include "CShader.h"
#include "CTexture.h"
#include "CTransform.h"
#include "CCollider2D.h"
#include "Entity.h"
#include "EntityManager.h"
#include <algorithm>
#include <spdlog/spdlog.h>

SRenderer::SRenderer() : System()
{
}

SRenderer::~SRenderer()
{
    shutdown();
}

SRenderer& SRenderer::instance()
{
    static SRenderer instance;
    return instance;
}

bool SRenderer::initialize(const WindowConfig& config)
{
    if (m_initialized)
    {
        spdlog::warn("SRenderer: Already initialized");
        return true;
    }

    // Create the window
    m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(config.width, config.height), config.title,
                                                   config.getStyleFlags(), config.getContextSettings());

    if (!m_window)
    {
        spdlog::error("SRenderer: Failed to create render window");
        return false;
    }

    // Apply window settings
    m_window->setVerticalSyncEnabled(config.vsync);
    if (config.frameLimit > 0)
    {
        m_window->setFramerateLimit(config.frameLimit);
    }

    m_initialized = true;
    spdlog::info("SRenderer: Initialized with window size {}x{}", config.width, config.height);
    return true;
}

void SRenderer::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    clearTextureCache();
    clearShaderCache();

    if (m_window)
    {
        m_window->close();
        m_window.reset();
    }

    m_initialized = false;
    spdlog::info("SRenderer: Shutdown complete");
}

void SRenderer::update(float deltaTime)
{
    // Rendering system doesn't need per-frame updates
    // Actual rendering is done in render()
}

void SRenderer::render()
{
    if (!m_initialized || !m_window || !m_window->isOpen())
    {
        return;
    }

    // Get all entities with renderable components
    auto& entityManager      = EntityManager::instance();
    auto  renderableEntities = entityManager.getEntitiesWithComponent<CRenderable>();

    // Sort entities by z-index (lower values draw first, higher on top)
    std::sort(renderableEntities.begin(), renderableEntities.end(), [](Entity* a, Entity* b) {
        auto* renderableA = a->getComponent<CRenderable>();
        auto* renderableB = b->getComponent<CRenderable>();

        if (!renderableA)
            return false;
        if (!renderableB)
            return true;

        return renderableA->getZIndex() < renderableB->getZIndex();
    });

    // Render each entity
    for (Entity* entity : renderableEntities)
    {
        renderEntity(entity);
    }
}

void SRenderer::clear(const Color& color)
{
    if (m_window && m_window->isOpen())
    {
        m_window->clear(color.toSFML());
    }
}

void SRenderer::display()
{
    if (m_window && m_window->isOpen())
    {
        m_window->display();
    }
}

bool SRenderer::isWindowOpen() const
{
    return m_window && m_window->isOpen();
}

sf::RenderWindow* SRenderer::getWindow()
{
    return m_window.get();
}

const sf::Texture* SRenderer::loadTexture(const std::string& filepath)
{
    if (filepath.empty())
    {
        return nullptr;
    }

    // Check if texture is already cached
    auto it = m_textureCache.find(filepath);
    if (it != m_textureCache.end())
    {
        return &it->second;
    }

    // Load new texture
    sf::Texture texture;
    if (!texture.loadFromFile(filepath))
    {
        spdlog::error("SRenderer: Failed to load texture from '{}'", filepath);
        return nullptr;
    }

    // Cache the texture
    m_textureCache[filepath] = std::move(texture);
    spdlog::debug("SRenderer: Loaded texture '{}'", filepath);
    return &m_textureCache[filepath];
}

const sf::Shader* SRenderer::loadShader(const std::string& vertexPath, const std::string& fragmentPath)
{
    if (vertexPath.empty() && fragmentPath.empty())
    {
        return nullptr;
    }

    // Create cache key from both paths
    std::string cacheKey = vertexPath + "|" + fragmentPath;

    // Check if shader is already cached
    auto it = m_shaderCache.find(cacheKey);
    if (it != m_shaderCache.end())
    {
        return it->second.get();
    }

    // Check if shaders are supported
    if (!sf::Shader::isAvailable())
    {
        spdlog::warn("SRenderer: Shaders are not available on this system");
        return nullptr;
    }

    // Load new shader
    auto shader = std::make_unique<sf::Shader>();
    bool loaded = false;

    if (!vertexPath.empty() && !fragmentPath.empty())
    {
        loaded = shader->loadFromFile(vertexPath, fragmentPath);
    }
    else if (!vertexPath.empty())
    {
        loaded = shader->loadFromFile(vertexPath, sf::Shader::Vertex);
    }
    else
    {
        loaded = shader->loadFromFile(fragmentPath, sf::Shader::Fragment);
    }

    if (!loaded)
    {
        spdlog::error("SRenderer: Failed to load shader (vertex: '{}', fragment: '{}')", vertexPath, fragmentPath);
        return nullptr;
    }

    // Cache the shader
    const sf::Shader* shaderPtr = shader.get();
    m_shaderCache[cacheKey] = std::move(shader);
    spdlog::debug("SRenderer: Loaded shader (vertex: '{}', fragment: '{}')", vertexPath, fragmentPath);
    return shaderPtr;
}

void SRenderer::clearTextureCache()
{
    m_textureCache.clear();
    spdlog::debug("SRenderer: Texture cache cleared");
}

void SRenderer::clearShaderCache()
{
    m_shaderCache.clear();
    spdlog::debug("SRenderer: Shader cache cleared");
}

void SRenderer::renderEntity(Entity* entity)
{
    if (!entity)
    {
        return;
    }

    // Check for required components
    auto* renderable = entity->getComponent<CRenderable>();
    auto* transform  = entity->getComponent<CTransform>();

    if (!renderable || !renderable->isActive() || !renderable->isVisible())
    {
        return;
    }

    if (!transform)
    {
        spdlog::warn("SRenderer: Entity has CRenderable but no CTransform");
        return;
    }

    // Get position, scale, and rotation
    Vec2  pos      = transform->getPosition();
    Vec2  scale    = transform->getScale();
    float rotation = transform->getRotation();

    // Get material if available
    auto* material = entity->getComponent<CMaterial>();

    // Determine final color
    Color finalColor = renderable->getColor();
    if (material)
    {
        // Apply material tint
        Color tint = material->getTint();
        finalColor.r = static_cast<uint8_t>((finalColor.r * tint.r) / 255);
        finalColor.g = static_cast<uint8_t>((finalColor.g * tint.g) / 255);
        finalColor.b = static_cast<uint8_t>((finalColor.b * tint.b) / 255);
        finalColor.a = static_cast<uint8_t>((finalColor.a * material->getOpacity()));
    }

    // Get texture if available
    const sf::Texture* texture = nullptr;
    if (material)
    {
        // Try to find texture component by GUID
        std::string textureGuid = material->getTextureGuid();
        if (!textureGuid.empty())
        {
            auto* textureComp = entity->getComponent<CTexture>();
            if (textureComp && textureComp->getGuid() == textureGuid)
            {
                texture = loadTexture(textureComp->getTexturePath());
            }
        }
    }

    // Get shader if available
    const sf::Shader* shader = nullptr;
    if (material)
    {
        std::string shaderGuid = material->getShaderGuid();
        if (!shaderGuid.empty())
        {
            auto* shaderComp = entity->getComponent<CShader>();
            if (shaderComp && shaderComp->getGuid() == shaderGuid)
            {
                shader = loadShader(shaderComp->getVertexShaderPath(), shaderComp->getFragmentShaderPath());
            }
        }
    }

    // Prepare render states
    sf::RenderStates states;
    if (material)
    {
        states.blendMode = toSFMLBlendMode(material->getBlendMode());
    }
    if (shader)
    {
        states.shader = shader;
    }

    // Render based on visual type
    VisualType visualType = renderable->getVisualType();

    switch (visualType)
    {
    case VisualType::Rectangle: {
        sf::RectangleShape rect;
        
        // If we have a collider, use its size
        auto* collider = entity->getComponent<CCollider2D>();
        if (collider && collider->getShapeType() == ColliderShape::Box)
        {
            float halfWidth = collider->getBoxHalfWidth();
            float halfHeight = collider->getBoxHalfHeight();
            rect.setSize(sf::Vector2f(halfWidth * 2.0f, halfHeight * 2.0f));
            rect.setOrigin(halfWidth, halfHeight);
        }
        else
        {
            // Default rectangle size
            rect.setSize(sf::Vector2f(50.0f * scale.x, 50.0f * scale.y));
            rect.setOrigin(25.0f * scale.x, 25.0f * scale.y);
        }

        rect.setPosition(pos.x, pos.y);
        rect.setRotation(rotation * 180.0f / 3.14159265f);  // Convert radians to degrees
        rect.setFillColor(finalColor.toSFML());

        if (texture)
        {
            rect.setTexture(texture);
        }

        m_window->draw(rect, states);
        break;
    }

    case VisualType::Circle: {
        sf::CircleShape circle;
        
        // If we have a collider, use its radius
        auto* collider = entity->getComponent<CCollider2D>();
        float radius = 25.0f;
        if (collider && collider->getShapeType() == ColliderShape::Circle)
        {
            radius = collider->getCircleRadius();
        }

        circle.setRadius(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(pos.x, pos.y);
        circle.setScale(scale.x, scale.y);
        circle.setRotation(rotation * 180.0f / 3.14159265f);
        circle.setFillColor(finalColor.toSFML());

        if (texture)
        {
            circle.setTexture(texture);
        }

        m_window->draw(circle, states);
        break;
    }

    case VisualType::Sprite: {
        if (texture)
        {
            sf::Sprite sprite(*texture);
            sf::FloatRect bounds = sprite.getLocalBounds();
            sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
            sprite.setPosition(pos.x, pos.y);
            sprite.setScale(scale.x, scale.y);
            sprite.setRotation(rotation * 180.0f / 3.14159265f);
            sprite.setColor(finalColor.toSFML());

            m_window->draw(sprite, states);
        }
        else
        {
            // Fallback: draw a rectangle if no texture
            sf::RectangleShape rect(sf::Vector2f(50.0f * scale.x, 50.0f * scale.y));
            rect.setOrigin(25.0f * scale.x, 25.0f * scale.y);
            rect.setPosition(pos.x, pos.y);
            rect.setRotation(rotation * 180.0f / 3.14159265f);
            rect.setFillColor(finalColor.toSFML());
            m_window->draw(rect, states);
        }
        break;
    }

    case VisualType::Custom:
    case VisualType::None:
    default:
        // No rendering for None or Custom (Custom would use shaders)
        break;
    }
}

sf::BlendMode SRenderer::toSFMLBlendMode(BlendMode blendMode) const
{
    switch (blendMode)
    {
    case BlendMode::Add:
        return sf::BlendAdd;
    case BlendMode::Multiply:
        return sf::BlendMultiply;
    case BlendMode::None:
        return sf::BlendNone;
    case BlendMode::Alpha:
    default:
        return sf::BlendAlpha;
    }
}
