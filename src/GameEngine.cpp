#include "GameEngine.h"
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// Include all component types for registry registration
#include <Components.h>

GameEngine::GameEngine(const Systems::WindowConfig& windowConfig, Vec2 gravity, uint8_t subStepCount, float timeStep, float pixelsPerMeter)
    : m_gravity(gravity), m_subStepCount(subStepCount), m_timeStep(timeStep)
{
    // Register component type names for serialization
    registerComponentTypes();

    // Initialize spdlog logger (using synchronous logging for now)
    if (!spdlog::get("GameEngine"))
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>("game_engine.log", true);

        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [Thread:%t] %v");
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [Thread:%t] %v");

        console_sink->set_level(spdlog::level::info);  // Reduce logging level
        file_sink->set_level(spdlog::level::debug);

        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        auto logger = std::make_shared<spdlog::logger>("GameEngine", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::info);  // Set to info to reduce spam
        spdlog::register_logger(logger);
    }

    if (auto logger = spdlog::get("GameEngine"))
    {
        logger->info("GameEngine initialized");
        logger->info("Window size: {}x{}", windowConfig.width, windowConfig.height);
        logger->info("SubSteps: {}, TimeStep: {}", (int)subStepCount, timeStep);
    }

    // Initialize renderer
    if (!::Systems::SRenderer::instance().initialize(windowConfig))
    {
        if (auto logger = spdlog::get("GameEngine"))
        {
            logger->error("Failed to initialize SRenderer");
        }
        return;
    }

    m_gameRunning = true;

    // Configure physics system
    auto& physics = ::Systems::S2DPhysics::instance();
    physics.setGravity({gravity.x, gravity.y});  // Box2D uses Y-up convention
    // Use the stored member values so the members are read/used and
    // do not trigger "unused member" style warnings from static analyzers
    physics.setSubStepCount(m_subStepCount);  // Set substep count for stability
    physics.setTimeStep(m_timeStep);          // Set fixed timestep

    // Initialize input manager and register window event handling
    ::Systems::SInput::instance().initialize(::Systems::SRenderer::instance().getWindow(), true);
    ::Systems::SInput::instance().subscribe(
        [this](const InputEvent& ev)
        {
            if (ev.type == InputEventType::WindowClosed)
            {
                auto* window = ::Systems::SRenderer::instance().getWindow();
                if (window)
                {
                    window->close();
                }
                m_gameRunning = false;
            }
        });

    // Initialize audio system
    ::Systems::SAudio::instance().initialize();

    // Initialize particle system with default scale
    // Note: Users can re-initialize with different scale if needed
    ::Systems::SParticle::instance().initialize(::Systems::SRenderer::instance().getWindow(), pixelsPerMeter);

    if (auto logger = spdlog::get("GameEngine"))
    {
        logger->info("All core systems initialized");
    }
}

GameEngine::~GameEngine()
{
    // Shutdown input manager
    ::Systems::SInput::instance().shutdown();

    // Shutdown renderer
    ::Systems::SRenderer::instance().shutdown();

    if (auto logger = spdlog::get("GameEngine"))
    {
        logger->info("GameEngine shutting down");
    }
    // Note: Don't drop the logger or shutdown thread pool here since the logger
    // may be reused if another GameEngine instance is created
}

void GameEngine::readInputs()
{
    // Intentionally empty - SInput polls SFML events and dispatches them.
}

void GameEngine::update(float deltaTime)
{
    // Handle input events before any updates
    ::Systems::SInput::instance().update(deltaTime);
    // Accumulate time for fixed timestep updates
    m_accumulator += deltaTime;

    // Run physics updates with fixed timestep (prevents spiral of death)
    const float maxAccumulator = m_timeStep * 10.0f;  // Cap to prevent spiral of death
    if (m_accumulator > maxAccumulator)
    {
        m_accumulator = maxAccumulator;
    }

    // Process fixed timestep updates
    while (m_accumulator >= m_timeStep)
    {
        // Run fixed-update callbacks for all physics bodies (apply forces, etc.)
        ::Systems::S2DPhysics::instance().runFixedUpdates(m_timeStep);

        // Box2D physics update (handles its own sub-stepping)
        ::Systems::S2DPhysics::instance().update(m_timeStep);

        m_accumulator -= m_timeStep;
    }

    // Update particle system with frame delta time
    ::Systems::SParticle::instance().update(deltaTime);
}

void GameEngine::render()
{
    auto& renderer = ::Systems::SRenderer::instance();
    renderer.clear(Color::Black);
    renderer.render();
    renderer.display();
}

bool GameEngine::is_running() const
{
    return m_gameRunning;
}

std::shared_ptr<spdlog::logger> GameEngine::getLogger()
{
    return spdlog::get("GameEngine");
}

Systems::SScene& GameEngine::getSceneManager()
{
    return ::Systems::SScene::instance();
}

Components::ComponentFactory& GameEngine::getComponentFactory()
{
    return ::Components::ComponentFactory::instance();
}

Systems::S2DPhysics& GameEngine::getPhysics()
{
    return ::Systems::S2DPhysics::instance();
}

Systems::SInput& GameEngine::getInputManager()
{
    return ::Systems::SInput::instance();
}

Systems::SAudio& GameEngine::getAudioSystem()
{
    return ::Systems::SAudio::instance();
}

Systems::SRenderer& GameEngine::getRenderer()
{
    return ::Systems::SRenderer::instance();
}

Systems::SParticle& GameEngine::getParticleSystem()
{
    return ::Systems::SParticle::instance();
}

void GameEngine::registerComponentTypes()
{
    // Register all component types with stable names for serialization
    // This ensures saves are portable and readable across builds
    using namespace Components;

    m_registry.registerTypeName<CTransform>("CTransform");
    m_registry.registerTypeName<CRenderable>("CRenderable");
    m_registry.registerTypeName<CPhysicsBody2D>("CPhysicsBody2D");
    m_registry.registerTypeName<CCollider2D>("CCollider2D");
    m_registry.registerTypeName<CMaterial>("CMaterial");
    m_registry.registerTypeName<CTexture>("CTexture");
    m_registry.registerTypeName<CShader>("CShader");
    m_registry.registerTypeName<CName>("CName");
    m_registry.registerTypeName<CInputController>("CInputController");
    m_registry.registerTypeName<CParticleEmitter>("CParticleEmitter");
    m_registry.registerTypeName<CAudioSource>("CAudioSource");
    m_registry.registerTypeName<CAudioListener>("CAudioListener");
}
