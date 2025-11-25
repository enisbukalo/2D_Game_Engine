#include "GameEngine.h"
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include "EntityManager.h"
#include "systems/SBox2DPhysics.h"

GameEngine::GameEngine(sf::RenderWindow* window, sf::Vector2f gravity, uint8_t subStepCount, float timeStep)
    : m_window(window), m_gravity(gravity), m_subStepCount(subStepCount), m_timeStep(timeStep)
{
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
        logger->info("Window size: {}x{}", window->getSize().x, window->getSize().y);
        logger->info("SubSteps: {}, TimeStep: {}", (int)subStepCount, timeStep);
    }

    m_gameRunning = true;

    // Configure physics system
    auto& physics = SBox2DPhysics::instance();
    physics.setGravity({gravity.x, gravity.y});  // Box2D uses Y-up convention
    // Use the stored member values so the members are read/used and
    // do not trigger "unused member" style warnings from static analyzers
    physics.setSubStepCount(m_subStepCount);  // Set substep count for stability
    physics.setTimeStep(m_timeStep);          // Set fixed timestep
}

GameEngine::~GameEngine()
{
    if (auto logger = spdlog::get("GameEngine"))
    {
        logger->info("GameEngine shutting down");
    }
    // Note: Don't drop the logger or shutdown thread pool here since the logger
    // may be reused if another GameEngine instance is created
}

void GameEngine::readInputs()
{
    sf::Event event;
    while (m_window->pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_window->close();
            m_gameRunning = false;
        }
    }
}

void GameEngine::update(float deltaTime)
{
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
        // Box2D physics update (handles its own sub-stepping)
        SBox2DPhysics::instance().update(m_timeStep);

        m_accumulator -= m_timeStep;
    }

    // Update entity manager with the actual frame delta time
    EntityManager::instance().update(deltaTime);
}

void GameEngine::render()
{
    m_window->clear();
    // Render game objects
    m_window->display();
}

bool GameEngine::is_running() const
{
    return m_gameRunning;
}

std::shared_ptr<spdlog::logger> GameEngine::getLogger()
{
    return spdlog::get("GameEngine");
}