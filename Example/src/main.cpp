#include <Entity.h>
#include <EntityManager.h>
#include <GameEngine.h>
#include <Vec2.h>
#include <components/CBoxCollider.h>
#include <components/CCircleCollider.h>
#include <components/CGravity.h>
#include <components/CTransform.h>
#include <utility/Logger.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <sstream>

class BounceGame
{
private:
    sf::RenderWindow            m_window;
    std::unique_ptr<GameEngine> m_gameEngine;
    sf::Font                    m_font;
    uint8_t                     m_subStepCount;
    int                         m_ballAmount;
    bool                        m_running;
    bool                        m_fontLoaded;
    bool                        m_gravityEnabled;

    const float RESTITUTION                 = 0.8f;      // Bounciness factor
    const float GRAVITY                     = 500.0f;    // Gravity force
    const float TIME_STEP                   = 0.01667f;  // Fixed time step for physics updates
    const int   BOUNDARY_COLLIDER_THICKNESS = 50;        // Thickness of boundary colliders

    // Define Screen Size
    const int SCREEN_WIDTH  = 1200;
    const int SCREEN_HEIGHT = 800;

public:
    BounceGame()
        : m_window(sf::VideoMode(1200, 800), "Bouncing Balls Example"),
          m_gameEngine(std::make_unique<GameEngine>(&m_window, sf::Vector2f(0.0f, 500.0f), 1, 0.01667f)),
          m_running(true),
          m_subStepCount(1),
          m_ballAmount(6),
          m_fontLoaded(false),
          m_gravityEnabled(true)
    {
        m_window.setFramerateLimit(60);

        // Try to load a system font (optional, will work without it)
        if (!m_font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            LOG_WARNING("Could not load font. UI text will not be displayed.");
        }
        else
        {
            m_fontLoaded = true;
        }
    }

    void init()
    {
        createBoundaryColliders();

        createBalls();

        // Force EntityManager to process pending entities
        EntityManager::instance().update(0.0f);

        spdlog::get("GameEngine")->info("Game initialized!");
        spdlog::get("GameEngine")->info("Controls:");
        spdlog::get("GameEngine")->info("  Up/Down or +/-  : Adjust physics substeps");
        spdlog::get("GameEngine")->info("  Left/Right      : Adjust ball count");
        spdlog::get("GameEngine")->info("  R               : Restart scenario");
        spdlog::get("GameEngine")->info("  G               : Toggle gravity");
        spdlog::get("GameEngine")->info("  Escape          : Exit");
        spdlog::get("GameEngine")->info("Initial SubSteps: {}", (int)m_subStepCount);
        spdlog::get("GameEngine")->info("Number of balls: {}", m_ballAmount);
        spdlog::get("GameEngine")->info("Gravity: {}", (m_gravityEnabled ? "ON" : "OFF"));

        // Debug: Check boundary colliders
        spdlog::get("GameEngine")->info("\nBoundary Colliders:");
        auto floor     = EntityManager::instance().getEntitiesByTag("floor");
        auto rightWall = EntityManager::instance().getEntitiesByTag("rightWall");
        auto leftWall  = EntityManager::instance().getEntitiesByTag("leftWall");
        auto topWall   = EntityManager::instance().getEntitiesByTag("topWall");

        if (!floor.empty())
        {
            auto* t = floor[0]->getComponent<CTransform>();
            auto* c = floor[0]->getComponent<CBoxCollider>();
            spdlog::get("GameEngine")
                ->info("Floor: pos({},{}) size({},{}) static={}",
                       t->getPosition().x,
                       t->getPosition().y,
                       c->getSize().x,
                       c->getSize().y,
                       c->isStatic());
        }
        if (!rightWall.empty())
        {
            auto* t = rightWall[0]->getComponent<CTransform>();
            auto* c = rightWall[0]->getComponent<CBoxCollider>();
            spdlog::get("GameEngine")
                ->info("RightWall: pos({},{}) size({},{}) static={}",
                       t->getPosition().x,
                       t->getPosition().y,
                       c->getSize().x,
                       c->getSize().y,
                       c->isStatic());
        }
        if (!leftWall.empty())
        {
            auto* t = leftWall[0]->getComponent<CTransform>();
            auto* c = leftWall[0]->getComponent<CBoxCollider>();
            spdlog::get("GameEngine")
                ->info("LeftWall: pos({},{}) size({},{}) static={}",
                       t->getPosition().x,
                       t->getPosition().y,
                       c->getSize().x,
                       c->getSize().y,
                       c->isStatic());
        }
        if (!topWall.empty())
        {
            auto* t = topWall[0]->getComponent<CTransform>();
            auto* c = topWall[0]->getComponent<CBoxCollider>();
            spdlog::get("GameEngine")
                ->info("TopWall: pos({},{}) size({},{}) static={}",
                       t->getPosition().x,
                       t->getPosition().y,
                       c->getSize().x,
                       c->getSize().y,
                       c->isStatic());
        }
    }

    void handleEvents()
    {
        sf::Event event;
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                m_running = false;
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    m_running = false;
                }
                // Use Up/Down arrow keys or +/- to adjust substeps
                else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Add
                         || event.key.code == sf::Keyboard::Equal)
                {
                    if (m_subStepCount < 8)
                    {
                        m_subStepCount++;
                        recreateGameEngine();
                        spdlog::get("GameEngine")->info("SubSteps: {}", (int)m_subStepCount);
                    }
                }
                else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Subtract
                         || event.key.code == sf::Keyboard::Hyphen)
                {
                    if (m_subStepCount > 1)
                    {
                        m_subStepCount--;
                        recreateGameEngine();
                        spdlog::get("GameEngine")->info("SubSteps: {}", (int)m_subStepCount);
                    }
                }
                // Adjust ball count with Left/Right arrows
                else if (event.key.code == sf::Keyboard::Left)
                {
                    if (m_ballAmount > 1)
                    {
                        m_ballAmount--;
                        spdlog::get("GameEngine")->info("Ball count: {} (Press R to restart)", m_ballAmount);
                    }
                }
                else if (event.key.code == sf::Keyboard::Right)
                {
                    if (m_ballAmount < 500)
                    {
                        m_ballAmount++;
                        spdlog::get("GameEngine")->info("Ball count: {} (Press R to restart)", m_ballAmount);
                    }
                }
                // Restart scenario with R key
                else if (event.key.code == sf::Keyboard::R)
                {
                    restart();
                }
                // Toggle gravity with G key
                else if (event.key.code == sf::Keyboard::G)
                {
                    toggleGravity();
                }
            }
        }
    }

    void createBoundaryColliders()
    {
        // Create boundary collider entities
        auto floor     = EntityManager::instance().addEntity("floor");
        auto rightWall = EntityManager::instance().addEntity("rightWall");
        auto leftWall  = EntityManager::instance().addEntity("leftWall");
        auto topWall   = EntityManager::instance().addEntity("topWall");

        // Position the boundary colliders around the screen edges
        floor->addComponent<CTransform>(Vec2(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - BOUNDARY_COLLIDER_THICKNESS / 2.0f),
                                        Vec2(1.0f, 1.0f),
                                        0.0f);
        rightWall->addComponent<CTransform>(Vec2(SCREEN_WIDTH - BOUNDARY_COLLIDER_THICKNESS / 2.0f, SCREEN_HEIGHT / 2.0f),
                                            Vec2(1.0f, 1.0f),
                                            0.0f);
        leftWall->addComponent<CTransform>(Vec2(BOUNDARY_COLLIDER_THICKNESS / 2.0f, SCREEN_HEIGHT / 2.0f), Vec2(1.0f, 1.0f), 0.0f);
        topWall->addComponent<CTransform>(Vec2(SCREEN_WIDTH / 2.0f, BOUNDARY_COLLIDER_THICKNESS / 2.0f), Vec2(1.0f, 1.0f), 0.0f);

        // Create boundary colliders (after transforms exist)
        auto* floorCollider = floor->addComponent<CBoxCollider>(SCREEN_WIDTH, BOUNDARY_COLLIDER_THICKNESS);
        auto* rightCollider = rightWall->addComponent<CBoxCollider>(BOUNDARY_COLLIDER_THICKNESS, SCREEN_HEIGHT);
        auto* leftCollider  = leftWall->addComponent<CBoxCollider>(BOUNDARY_COLLIDER_THICKNESS, SCREEN_HEIGHT);
        auto* topCollider   = topWall->addComponent<CBoxCollider>(SCREEN_WIDTH, BOUNDARY_COLLIDER_THICKNESS);

        // Mark walls and floor as static
        floorCollider->setStatic(true);
        rightCollider->setStatic(true);
        leftCollider->setStatic(true);
        topCollider->setStatic(true);
    }

    void createBalls()
    {
        for (int i = 0; i < m_ballAmount; i++)
        {
            // Lets get a random X and Y position within ranges of
            // X between 450 and 750
            // Y between 50 and 500
            float randomX = static_cast<float>(rand() % (750 - 450 + 1) + 450);
            float randomY = static_cast<float>(rand() % (500 - 50 + 1) + 50);
            auto  ball    = EntityManager::instance().addEntity("ball");
            ball->addComponent<CTransform>(Vec2(randomX, randomY), Vec2(1.0f, 1.0f), 0.0f);
            ball->addComponent<CCircleCollider>(20.0f);
            auto* gravity1 = ball->addComponent<CGravity>();
            gravity1->setForce(Vec2(0.0f, GRAVITY));  // Positive Y = downward

            // Randomize initial velocity
            auto* transform   = ball->getComponent<CTransform>();
            float initialVelX = static_cast<float>((rand() % 501) - 100);  // -100 to +100
            float initialVelY = static_cast<float>((rand() % 501) - 100);  // -100 to +100
            transform->setVelocity(Vec2(initialVelX, initialVelY));
        }
    }

    void toggleGravity()
    {
        m_gravityEnabled = !m_gravityEnabled;

        // Update gravity on all balls
        auto balls = EntityManager::instance().getEntitiesByTag("ball");
        for (auto& ball : balls)
        {
            if (ball->hasComponent<CGravity>())
            {
                auto* gravity = ball->getComponent<CGravity>();
                if (m_gravityEnabled)
                {
                    gravity->setForce(Vec2(0.0f, GRAVITY));
                }
                else
                {
                    gravity->setForce(Vec2(0.0f, 0.0f));
                }
            }
        }

        spdlog::get("GameEngine")->info("Gravity: {}", (m_gravityEnabled ? "ON" : "OFF"));
    }

    void recreateGameEngine()
    {
        // Recreate the game engine with new substep count
        m_gameEngine = std::make_unique<GameEngine>(&m_window, sf::Vector2f(0.0f, GRAVITY), m_subStepCount, TIME_STEP);
    }

    void restart()
    {
        spdlog::get("GameEngine")->info("\n=== Restarting scenario ===");
        spdlog::get("GameEngine")->info("Ball count: {}", m_ballAmount);
        spdlog::get("GameEngine")->info("SubSteps: {}", (int)m_subStepCount);
        spdlog::get("GameEngine")->info("Gravity: {}", (m_gravityEnabled ? "ON" : "OFF"));

        // Clear all entities
        EntityManager::instance().clear();

        // Recreate game engine
        m_gameEngine = std::make_unique<GameEngine>(&m_window, sf::Vector2f(0.0f, GRAVITY), m_subStepCount, TIME_STEP);

        // Recreate boundary colliders and balls
        createBoundaryColliders();
        createBalls();

        // Force EntityManager to process pending entities
        EntityManager::instance().update(0.0f);

        spdlog::get("GameEngine")->info("=== Restart complete ===");
    }

    void update(float dt)
    {
        // GameEngine handles physics updates with fixed timestep
        m_gameEngine->update(dt);
    }

    void render()
    {
        m_window.clear(sf::Color(50, 50, 50));  // Dark gray background

        // Draw boundary colliders
        std::vector<std::string> boundaryTags = {"floor", "rightWall", "leftWall", "topWall"};
        for (const auto& tag : boundaryTags)
        {
            auto boundaries = EntityManager::instance().getEntitiesByTag(tag);
            for (auto& boundaryCollider : boundaries)
            {
                if (!boundaryCollider->hasComponent<CTransform>() || !boundaryCollider->hasComponent<CBoxCollider>())
                    continue;

                auto* transform = boundaryCollider->getComponent<CTransform>();
                auto* collider  = boundaryCollider->getComponent<CBoxCollider>();

                Vec2 pos  = transform->getPosition();
                Vec2 size = collider->getSize();

                sf::RectangleShape shape(sf::Vector2f(size.x, size.y));
                shape.setOrigin(size.x / 2, size.y / 2);
                shape.setPosition(pos.x, pos.y);
                shape.setFillColor(sf::Color(100, 100, 100));  // Gray
                m_window.draw(shape);
            }
        }

        // Draw balls
        auto balls     = EntityManager::instance().getEntitiesByTag("ball");
        int  ballIndex = 0;
        for (auto& ball : balls)
        {
            if (!ball->hasComponent<CTransform>() || !ball->hasComponent<CCircleCollider>())
                continue;

            auto* transform = ball->getComponent<CTransform>();
            auto* collider  = ball->getComponent<CCircleCollider>();

            Vec2  pos    = transform->getPosition();
            float radius = collider->getRadius();

            sf::CircleShape ballShape(radius);
            ballShape.setOrigin(radius, radius);
            ballShape.setPosition(pos.x, pos.y);

            // Set a random color for each ball
            sf::Color color;
            switch (ballIndex % 5)
            {
                case 0:
                    color = sf::Color::Red;
                    break;
                case 1:
                    color = sf::Color::Green;
                    break;
                case 2:
                    color = sf::Color::Blue;
                    break;
                case 3:
                    color = sf::Color::Yellow;
                    break;
                case 4:
                    color = sf::Color::Cyan;
                    break;
            }
            ballShape.setFillColor(color);

            m_window.draw(ballShape);
            ballIndex++;
        }

        // Draw UI text showing current substep count and gravity status
        if (m_fontLoaded)
        {
            std::ostringstream oss;
            oss << "SubSteps: " << (int)m_subStepCount << " (Use Up/Down or +/-)\n";
            oss << "Ball Count: " << m_ballAmount << " (Use Left/Right, press R to restart)\n";
            oss << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << " (Press G to toggle)";

            sf::Text text;
            text.setFont(m_font);
            text.setString(oss.str());
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::White);
            text.setPosition(10.0f, 10.0f);
            m_window.draw(text);
        }

        m_window.display();
    }

    void run()
    {
        init();

        sf::Clock clock;
        clock.restart();  // Reset clock after init to get proper first frame delta
        while (m_running && m_window.isOpen())
        {
            float dt = clock.restart().asSeconds();

            handleEvents();
            update(dt);
            render();
        }

        m_window.close();
    }
};

int main()
{
    try
    {
        BounceGame game;
        game.run();
    }
    catch (const std::exception& e)
    {
        spdlog::get("GameEngine")->error("Fatal error: {}", e.what());
        return 1;
    }

    return 0;
}
