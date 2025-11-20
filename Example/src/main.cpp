#include <Entity.h>
#include <EntityManager.h>
#include <GameEngine.h>
#include <Vec2.h>
#include <components/CBoxCollider.h>
#include <components/CCircleCollider.h>
#include <components/CForceDebug.h>
#include <components/CGravity.h>
#include <components/CTransform.h>
#include <physics/Quadtree.h>
#include <systems/S2DPhysics.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <sstream>

// Define Screen Size
const int   SCREEN_WIDTH            = 1600;
const int   SCREEN_HEIGHT           = 1000;
const int   INITIAL_BALL_COUNT      = 125;
const int   INITIAL_SUBSTEP_COUNT   = 4;
const bool  INITIAL_GRAVITY_ENABLED = false;
const float TIME_STEP               = 0.01667f;  // 60 FPS
const float GRAVITY_FORCE           = 981.0f;    // Gravity force (9.81 m/s² at 100px/m scale)

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
    bool                        m_showColliders;
    bool                        m_showVectors;

    const float RESTITUTION                 = 0.8f;   // Bounciness factor
    const float BALL_RADIUS                 = 10.0f;  // Radius of each ball
    const int   BOUNDARY_COLLIDER_THICKNESS = 50;     // Thickness of boundary colliders

public:
    BounceGame()
        : m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Bouncing Balls Example"),
          m_gameEngine(
              std::make_unique<GameEngine>(&m_window, sf::Vector2f(0.0f, INITIAL_GRAVITY_ENABLED ? GRAVITY_FORCE : 0.0f), 1, TIME_STEP)),
          m_running(true),
          m_subStepCount(INITIAL_SUBSTEP_COUNT),
          m_ballAmount(INITIAL_BALL_COUNT),
          m_fontLoaded(false),
          m_gravityEnabled(INITIAL_GRAVITY_ENABLED),
          m_showColliders(true),
          m_showVectors(false)
    {
        m_window.setFramerateLimit(60);

        // Try to load a system font (optional, will work without it)
        if (!m_font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
        {
            std::cout << "Could not load font. UI text will not be displayed." << std::endl;
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

        std::cout << "Game initialized!" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  Up/Down or +/-  : Adjust physics substeps" << std::endl;
        std::cout << "  Left/Right      : Adjust ball count" << std::endl;
        std::cout << "  R               : Restart scenario" << std::endl;
        std::cout << "  G               : Toggle gravity" << std::endl;
        std::cout << "  C               : Toggle collider visibility" << std::endl;
        std::cout << "  V               : Toggle vector visualization" << std::endl;
        std::cout << "  Escape          : Exit" << std::endl;
        std::cout << "Initial SubSteps: " << (int)m_subStepCount << std::endl;
        std::cout << "Number of balls: " << m_ballAmount << std::endl;
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;
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
                        std::cout << "SubSteps: " << (int)m_subStepCount << std::endl;
                    }
                }
                else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::Subtract
                         || event.key.code == sf::Keyboard::Hyphen)
                {
                    if (m_subStepCount > 1)
                    {
                        m_subStepCount--;
                        recreateGameEngine();
                        std::cout << "SubSteps: " << (int)m_subStepCount << std::endl;
                    }
                }
                // Adjust ball count with Left/Right arrows
                else if (event.key.code == sf::Keyboard::Left)
                {
                    if (m_ballAmount > 1)
                    {
                        m_ballAmount--;
                        removeRandomBall();
                        std::cout << "Ball count: " << m_ballAmount << std::endl;
                    }
                }
                else if (event.key.code == sf::Keyboard::Right)
                {
                    if (m_ballAmount < 500)
                    {
                        m_ballAmount++;
                        spawnRandomBall();
                        std::cout << "Ball count: " << m_ballAmount << std::endl;
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
                // Toggle collider visibility with C key
                else if (event.key.code == sf::Keyboard::C)
                {
                    toggleColliders();
                }
                // Toggle vector visualization with V key
                else if (event.key.code == sf::Keyboard::V)
                {
                    toggleVectors();
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
        // Calculate spawn boundaries accounting for boundary thickness and ball radius
        const float MIN_X = BOUNDARY_COLLIDER_THICKNESS + BALL_RADIUS;
        const float MAX_X = SCREEN_WIDTH - BOUNDARY_COLLIDER_THICKNESS - BALL_RADIUS;
        const float MIN_Y = BOUNDARY_COLLIDER_THICKNESS + BALL_RADIUS;
        const float MAX_Y = SCREEN_HEIGHT - BOUNDARY_COLLIDER_THICKNESS - BALL_RADIUS;

        for (int i = 0; i < m_ballAmount; i++)
        {
            // Random position within safe spawn area
            float randomX = MIN_X + static_cast<float>(rand()) / RAND_MAX * (MAX_X - MIN_X);
            float randomY = MIN_Y + static_cast<float>(rand()) / RAND_MAX * (MAX_Y - MIN_Y);
            auto  ball    = EntityManager::instance().addEntity("ball");
            ball->addComponent<CTransform>(Vec2(randomX, randomY), Vec2(1.0f, 1.0f), 0.0f);
            ball->addComponent<CCircleCollider>(BALL_RADIUS);
            ball->addComponent<CGravity>();     // Uses default 1.0 multiplier
            ball->addComponent<CForceDebug>();  // For force visualization

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

        // Update global gravity in physics system
        if (m_gravityEnabled)
        {
            S2DPhysics::instance().setGlobalGravity(Vec2(0.0f, GRAVITY_FORCE));
        }
        else
        {
            S2DPhysics::instance().setGlobalGravity(Vec2(0.0f, 0.0f));
        }
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;
    }

    void toggleColliders()
    {
        m_showColliders = !m_showColliders;
        std::cout << "Colliders: " << (m_showColliders ? "ON" : "OFF") << std::endl;
    }

    void toggleVectors()
    {
        m_showVectors = !m_showVectors;
        std::cout << "Vectors: " << (m_showVectors ? "ON" : "OFF") << std::endl;
    }

    void spawnRandomBall()
    {
        // Calculate spawn boundaries accounting for boundary thickness and ball radius
        const float MIN_X = BOUNDARY_COLLIDER_THICKNESS + BALL_RADIUS;
        const float MAX_X = SCREEN_WIDTH - BOUNDARY_COLLIDER_THICKNESS - BALL_RADIUS;
        const float MIN_Y = BOUNDARY_COLLIDER_THICKNESS + BALL_RADIUS;
        const float MAX_Y = SCREEN_HEIGHT - BOUNDARY_COLLIDER_THICKNESS - BALL_RADIUS;

        // Random position within safe spawn area
        float randomX = MIN_X + static_cast<float>(rand()) / RAND_MAX * (MAX_X - MIN_X);
        float randomY = MIN_Y + static_cast<float>(rand()) / RAND_MAX * (MAX_Y - MIN_Y);
        auto  ball    = EntityManager::instance().addEntity("ball");
        ball->addComponent<CTransform>(Vec2(randomX, randomY), Vec2(1.0f, 1.0f), 0.0f);
        ball->addComponent<CCircleCollider>(BALL_RADIUS);
        ball->addComponent<CGravity>();     // Uses default 1.0 multiplier
        ball->addComponent<CForceDebug>();  // For force visualization

        // Randomize initial velocity
        auto* transform   = ball->getComponent<CTransform>();
        float initialVelX = static_cast<float>((rand() % 501) - 100);  // -100 to +100
        float initialVelY = static_cast<float>((rand() % 501) - 100);  // -100 to +100
        transform->setVelocity(Vec2(initialVelX, initialVelY));
    }

    void removeRandomBall()
    {
        auto balls = EntityManager::instance().getEntitiesByTag("ball");
        if (!balls.empty())
        {
            // Pick a random ball to remove
            int randomIndex = rand() % balls.size();
            balls[randomIndex]->destroy();
        }
    }

    void recreateGameEngine()
    {
        // Recreate the game engine with new substep count
        m_gameEngine = std::make_unique<GameEngine>(&m_window, sf::Vector2f(0.0f, m_gravityEnabled ? GRAVITY_FORCE : 0.0f), m_subStepCount, TIME_STEP);
    }

    void restart()
    {
        std::cout << "\n=== Restarting scenario ===" << std::endl;
        std::cout << "Ball count: " << m_ballAmount << std::endl;
        std::cout << "SubSteps: " << (int)m_subStepCount << std::endl;
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;

        // Clear all entities
        EntityManager::instance().clear();

        // Recreate game engine
        m_gameEngine = std::make_unique<GameEngine>(&m_window, sf::Vector2f(0.0f, m_gravityEnabled ? GRAVITY_FORCE : 0.0f), m_subStepCount, TIME_STEP);

        // Recreate boundary colliders and balls
        createBoundaryColliders();
        createBalls();

        // Force EntityManager to process pending entities
        EntityManager::instance().update(0.0f);

        std::cout << "=== Restart complete ===" << std::endl;
    }

    void drawVector(const Vec2& start, const Vec2& vector, const sf::Color& color, float scale = 1.0f)
    {
        // Calculate end point based on start position and vector
        Vec2 scaledVector = vector * scale;
        Vec2 end          = start + scaledVector;

        // Draw line using VertexArray
        sf::VertexArray line(sf::Lines, 2);
        line[0].position = sf::Vector2f(start.x, start.y);
        line[0].color    = color;
        line[1].position = sf::Vector2f(end.x, end.y);
        line[1].color    = color;

        m_window.draw(line);
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
                if (m_showColliders)
                {
                    shape.setOutlineColor(sf::Color(0, 255, 0));  // Lime green
                    shape.setOutlineThickness(2.0f);
                }
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
            if (m_showColliders)
            {
                ballShape.setOutlineColor(sf::Color(0, 255, 0));  // Lime green
                ballShape.setOutlineThickness(2.0f);
            }

            m_window.draw(ballShape);
            ballIndex++;
        }

        // Draw velocity and force vectors
        if (m_showVectors)
        {
            for (auto& ball : balls)
            {
                if (!ball->hasComponent<CTransform>())
                    continue;

                auto* transform = ball->getComponent<CTransform>();
                Vec2  pos       = transform->getPosition();

                // Draw velocity vector (yellow)
                Vec2 velocity = transform->getVelocity();
                if (velocity.length() > 0.01f)  // Only draw if velocity is non-negligible
                {
                    drawVector(pos, velocity, sf::Color::Yellow, 0.1f);
                }

                // Draw gravity force vector (red)
                auto* forceDebug = ball->getComponent<CForceDebug>();
                if (forceDebug)
                {
                    Vec2 gravityForce = forceDebug->getGravityForce();
                    if (gravityForce.length() > 0.01f)  // Only draw if force is non-negligible
                    {
                        drawVector(pos, gravityForce, sf::Color::Red, 0.01f);
                    }
                }
            }
        }

        // Draw quadtree visualization
        if (m_showColliders)
        {
            const Quadtree* quadtree = S2DPhysics::instance().getQuadtree();
            if (quadtree)
            {
                std::vector<AABB> quadtreeBounds;
                quadtree->getAllBounds(quadtreeBounds);

                for (const auto& bounds : quadtreeBounds)
                {
                    // Calculate the full size from half-size
                    float width  = bounds.halfSize.x * 2.0f;
                    float height = bounds.halfSize.y * 2.0f;

                    sf::RectangleShape quadShape(sf::Vector2f(width, height));
                    quadShape.setOrigin(width / 2.0f, height / 2.0f);
                    quadShape.setPosition(bounds.position.x, bounds.position.y);
                    quadShape.setFillColor(sf::Color::Transparent);
                    quadShape.setOutlineColor(sf::Color(255, 255, 0, 128));  // Semi-transparent yellow
                    quadShape.setOutlineThickness(1.0f);
                    m_window.draw(quadShape);
                }
            }
        }

        // Draw UI text showing current substep count and gravity status
        if (m_fontLoaded)
        {
            std::ostringstream oss;
            oss << "SubSteps: " << (int)m_subStepCount << " (Use Up/Down or +/-)\n";
            oss << "Ball Count: " << m_ballAmount << " (Use Left/Right to add/remove)\n";
            oss << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << " (Press G to toggle)\n";
            oss << "Colliders/Quadtree: " << (m_showColliders ? "ON" : "OFF") << " (Press C to toggle)\n";
            oss << "Vectors: " << (m_showVectors ? "ON" : "OFF") << " (Press V to toggle)";

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
        std::cout << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
