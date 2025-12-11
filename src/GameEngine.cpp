#include "GameEngine.h"
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// Include all component types for registry registration
#include <Components.h>
#include <SystemLocator.h>

GameEngine::GameEngine(const Systems::WindowConfig& windowConfig, Vec2 gravity, uint8_t subStepCount, float timeStep, float pixelsPerMeter)
    : m_gravity(gravity), m_subStepCount(subStepCount), m_timeStep(timeStep)
{
    m_renderer = std::make_unique<Systems::SRenderer>();
    m_input    = std::make_unique<Systems::SInput>();
    m_physics  = std::make_unique<Systems::S2DPhysics>();
    m_particle = std::make_unique<Systems::SParticle>();
    m_audio    = std::make_unique<Systems::SAudio>();

    Systems::SystemLocator::provideRenderer(m_renderer.get());
    Systems::SystemLocator::provideInput(m_input.get());
    Systems::SystemLocator::providePhysics(m_physics.get());
    Systems::SystemLocator::provideParticle(m_particle.get());
    Systems::SystemLocator::provideAudio(m_audio.get());

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
    if (!m_renderer->initialize(windowConfig))
    {
        if (auto logger = spdlog::get("GameEngine"))
        {
            logger->error("Failed to initialize SRenderer");
        }
        return;
    }

    m_renderer->setParticleSystem(m_particle.get());

    m_gameRunning = true;

    // Configure physics system
    m_physics->setGravity({gravity.x, gravity.y});  // Box2D uses Y-up convention
    // Use the stored member values so the members are read/used and
    // do not trigger "unused member" style warnings from static analyzers
    m_physics->setSubStepCount(m_subStepCount);  // Set substep count for stability
    m_physics->setTimeStep(m_timeStep);          // Set fixed timestep

    // Initialize input manager and register window event handling
    m_input->initialize(m_renderer->getWindow(), true);
    m_input->subscribe(
        [this](const InputEvent& ev)
        {
            if (ev.type == InputEventType::WindowClosed)
            {
                auto* window = m_renderer->getWindow();
                if (window)
                {
                    window->close();
                }
                m_gameRunning = false;
            }
        });

    // Initialize audio system
    m_audio->initialize();

    // Initialize particle system with default scale
    // Note: Users can re-initialize with different scale if needed
    m_particle->initialize(m_renderer->getWindow(), pixelsPerMeter);

    // Maintain ordered list for per-frame updates (input -> physics -> particle)
    m_systemOrder = {m_input.get(), m_physics.get(), m_particle.get()};

    if (auto logger = spdlog::get("GameEngine"))
    {
        logger->info("All core systems initialized");
    }
}

GameEngine::~GameEngine()
{
    // Shutdown input manager
    if (m_input)
    {
        m_input->shutdown();
    }

    // Shutdown renderer
    if (m_renderer)
    {
        m_renderer->shutdown();
    }

    if (m_particle)
    {
        m_particle->shutdown();
    }

    if (m_audio)
    {
        m_audio->shutdown();
    }

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
    for (auto* system : m_systemOrder)
    {
        if (!system)
        {
            continue;
        }

        if (system == m_physics.get())
        {
            m_accumulator += deltaTime;
            const float maxAccumulator = m_timeStep * 10.0f;  // Cap to prevent spiral of death
            if (m_accumulator > maxAccumulator)
            {
                m_accumulator = maxAccumulator;
            }

            while (m_accumulator >= m_timeStep)
            {
                m_physics->runFixedUpdates(m_timeStep);
                m_physics->update(m_timeStep, m_world);
                m_accumulator -= m_timeStep;
            }
            continue;
        }

        system->update(deltaTime, m_world);
    }

    // Apply deferred destroys after systems have finished updating to avoid iterator invalidation
    m_world.processDestroyQueue();

    if (m_audio)
    {
        m_audio->update(deltaTime);
    }
}

void GameEngine::render()
{
    if (!m_renderer)
    {
        return;
    }

    m_renderer->clear(Color::Black);
    m_renderer->render(m_world);
    m_renderer->display();
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

Systems::S2DPhysics& GameEngine::getPhysics()
{
    return *m_physics;
}

Systems::SInput& GameEngine::getInputManager()
{
    return *m_input;
}

Systems::SAudio& GameEngine::getAudioSystem()
{
    return *m_audio;
}

Systems::SRenderer& GameEngine::getRenderer()
{
    return *m_renderer;
}

Systems::SParticle& GameEngine::getParticleSystem()
{
    return *m_particle;
}

void GameEngine::registerComponentTypes()
{
    // Register all component types with stable names for serialization
    // This ensures saves are portable and readable across builds
    using namespace Components;

    m_world.registerTypeName<CTransform>("CTransform");
    m_world.registerTypeName<CRenderable>("CRenderable");
    m_world.registerTypeName<CPhysicsBody2D>("CPhysicsBody2D");
    m_world.registerTypeName<CCollider2D>("CCollider2D");
    m_world.registerTypeName<CMaterial>("CMaterial");
    m_world.registerTypeName<CTexture>("CTexture");
    m_world.registerTypeName<CShader>("CShader");
    m_world.registerTypeName<CName>("CName");
    m_world.registerTypeName<CInputController>("CInputController");
    m_world.registerTypeName<CParticleEmitter>("CParticleEmitter");
    m_world.registerTypeName<CAudioSource>("CAudioSource");
    m_world.registerTypeName<CAudioListener>("CAudioListener");
}
