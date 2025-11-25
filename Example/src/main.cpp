#include <Entity.h>
#include <EntityManager.h>
#include <GameEngine.h>
#include <Vec2.h>
#include <components/CCollider2D.h>
#include <components/CPhysicsBody2D.h>
#include <components/CTransform.h>
#include <systems/SBox2DPhysics.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <sstream>

// Define Screen Size
const int   SCREEN_WIDTH            = 1600;
const int   SCREEN_HEIGHT           = 1000;
const int   INITIAL_BALL_COUNT      = 500;
const bool  INITIAL_GRAVITY_ENABLED = false;
const float TIME_STEP               = 1.0f / 60.0f;  // 60 FPS
const float GRAVITY_FORCE           = -10.0f;        // Box2D gravity (m/s²), negative = downward
const float PIXELS_PER_METER        = 100.0f;        // Rendering scale: 100 pixels = 1 meter

class BounceGame
{
private:
    sf::RenderWindow            m_window;
    std::unique_ptr<GameEngine> m_gameEngine;
    sf::Font                    m_font;
    int                         m_ballAmount;
    bool                        m_running;
    bool                        m_fontLoaded;
    bool                        m_gravityEnabled;
    bool                        m_showColliders;
    bool                        m_showVectors;

    const float RESTITUTION                 = 0.8f;    // Bounciness factor
    const float BALL_RADIUS_METERS          = 0.1f;    // Radius in meters
    const float BOUNDARY_THICKNESS_METERS   = 0.5f;    // Thickness in meters
    const float RANDOM_VELOCITY_RANGE       = 2.0f;    // Random velocity range: -2 to +2 m/s

    // Helper function to convert meters to pixels for rendering
    sf::Vector2f metersToPixels(const Vec2& meters) const
    {
        // Note: Y-flip for screen coordinates (Box2D Y-up -> Screen Y-down)
        return sf::Vector2f(
            meters.x * PIXELS_PER_METER,
            SCREEN_HEIGHT - (meters.y * PIXELS_PER_METER)
        );
    }

    // Helper function to convert pixels to meters for physics
    Vec2 pixelsToMeters(float x, float y) const
    {
        // Note: Y-flip from screen coordinates to Box2D coordinates
        return Vec2(
            x / PIXELS_PER_METER,
            (SCREEN_HEIGHT - y) / PIXELS_PER_METER
        );
    }

    // Helper function to generate random velocity in a symmetric range
    Vec2 getRandomVelocity() const
    {
        float velX = static_cast<float>((rand() % static_cast<int>(RANDOM_VELOCITY_RANGE * 2000 + 1)) - RANDOM_VELOCITY_RANGE * 1000) / 1000.0f;
        float velY = static_cast<float>((rand() % static_cast<int>(RANDOM_VELOCITY_RANGE * 2000 + 1)) - RANDOM_VELOCITY_RANGE * 1000) / 1000.0f;
        return Vec2(velX, velY);
    }

public:
    BounceGame()
        : m_window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Bouncing Balls Example - Box2D"),
          m_gameEngine(std::make_unique<GameEngine>(&m_window, sf::Vector2f(0.0f, GRAVITY_FORCE))),
          m_running(true),
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
        // Set up Box2D physics world
        auto& physics = SBox2DPhysics::instance();
        physics.setGravity({0.0f, m_gravityEnabled ? GRAVITY_FORCE : 0.0f});

        createBoundaryColliders();
        createBalls();

        // Force EntityManager to process pending entities
        EntityManager::instance().update(0.0f);

        std::cout << "Game initialized!" << std::endl;
        std::cout << "Physics: Box2D v3.0 (1 unit = 1 meter, Y-up)" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  Left/Right      : Adjust ball count" << std::endl;
        std::cout << "  R               : Restart scenario" << std::endl;
        std::cout << "  G               : Toggle gravity" << std::endl;
        std::cout << "  C               : Toggle collider visibility" << std::endl;
        std::cout << "  V               : Toggle vector visualization" << std::endl;
        std::cout << "  Escape          : Exit" << std::endl;
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
                    if (m_ballAmount < 1000)
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
        // Screen dimensions in meters
        const float screenWidthMeters = SCREEN_WIDTH / PIXELS_PER_METER;
        const float screenHeightMeters = SCREEN_HEIGHT / PIXELS_PER_METER;

        // Create boundary collider entities
        auto floor     = EntityManager::instance().addEntity("floor");
        auto rightWall = EntityManager::instance().addEntity("rightWall");
        auto leftWall  = EntityManager::instance().addEntity("leftWall");
        auto topWall   = EntityManager::instance().addEntity("topWall");

        // Position the boundary colliders in meters (Box2D Y-up coordinates)
        floor->addComponent<CTransform>(Vec2(screenWidthMeters / 2.0f, BOUNDARY_THICKNESS_METERS / 2.0f), Vec2(1.0f, 1.0f), 0.0f);
        rightWall->addComponent<CTransform>(Vec2(screenWidthMeters - BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f), Vec2(1.0f, 1.0f), 0.0f);
        leftWall->addComponent<CTransform>(Vec2(BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f), Vec2(1.0f, 1.0f), 0.0f);
        topWall->addComponent<CTransform>(Vec2(screenWidthMeters / 2.0f, screenHeightMeters - BOUNDARY_THICKNESS_METERS / 2.0f), Vec2(1.0f, 1.0f), 0.0f);

        // Create physics bodies and colliders
        auto* floorBody = floor->addComponent<CPhysicsBody2D>();
        floorBody->initialize({screenWidthMeters / 2.0f, BOUNDARY_THICKNESS_METERS / 2.0f}, BodyType::Static);
        floor->addComponent<CCollider2D>()->createBox(screenWidthMeters / 2.0f, BOUNDARY_THICKNESS_METERS / 2.0f);

        auto* rightBody = rightWall->addComponent<CPhysicsBody2D>();
        rightBody->initialize({screenWidthMeters - BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f}, BodyType::Static);
        rightWall->addComponent<CCollider2D>()->createBox(BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f);

        auto* leftBody = leftWall->addComponent<CPhysicsBody2D>();
        leftBody->initialize({BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f}, BodyType::Static);
        leftWall->addComponent<CCollider2D>()->createBox(BOUNDARY_THICKNESS_METERS / 2.0f, screenHeightMeters / 2.0f);

        auto* topBody = topWall->addComponent<CPhysicsBody2D>();
        topBody->initialize({screenWidthMeters / 2.0f, screenHeightMeters - BOUNDARY_THICKNESS_METERS / 2.0f}, BodyType::Static);
        topWall->addComponent<CCollider2D>()->createBox(screenWidthMeters / 2.0f, BOUNDARY_THICKNESS_METERS / 2.0f);
    }

    void createBalls()
    {
        // Screen dimensions in meters
        const float screenWidthMeters = SCREEN_WIDTH / PIXELS_PER_METER;
        const float screenHeightMeters = SCREEN_HEIGHT / PIXELS_PER_METER;

        // Calculate spawn boundaries accounting for boundary thickness and ball radius
        const float MIN_X = BOUNDARY_THICKNESS_METERS + BALL_RADIUS_METERS;
        const float MAX_X = screenWidthMeters - BOUNDARY_THICKNESS_METERS - BALL_RADIUS_METERS;
        const float MIN_Y = BOUNDARY_THICKNESS_METERS + BALL_RADIUS_METERS;
        const float MAX_Y = screenHeightMeters - BOUNDARY_THICKNESS_METERS - BALL_RADIUS_METERS;

        for (int i = 0; i < m_ballAmount; i++)
        {
            // Random position within safe spawn area (in meters)
            float randomX = MIN_X + static_cast<float>(rand()) / RAND_MAX * (MAX_X - MIN_X);
            float randomY = MIN_Y + static_cast<float>(rand()) / RAND_MAX * (MAX_Y - MIN_Y);

            auto ball = EntityManager::instance().addEntity("ball");
            ball->addComponent<CTransform>(Vec2(randomX, randomY), Vec2(1.0f, 1.0f), 0.0f);

            auto* physicsBody = ball->addComponent<CPhysicsBody2D>();
            physicsBody->initialize({randomX, randomY}, BodyType::Dynamic);

            auto* collider = ball->addComponent<CCollider2D>();
            collider->createCircle(BALL_RADIUS_METERS);
            collider->setRestitution(RESTITUTION);

            // Randomize initial velocity
            physicsBody->setLinearVelocity({getRandomVelocity().x, getRandomVelocity().y});
        }
    }

    void toggleGravity()
    {
        m_gravityEnabled = !m_gravityEnabled;

        // Update global gravity in Box2D physics system
        auto& physics = SBox2DPhysics::instance();
        if (m_gravityEnabled)
        {
            physics.setGravity({0.0f, GRAVITY_FORCE});
        }
        else
        {
            physics.setGravity({0.0f, 0.0f});
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
        // Screen dimensions in meters
        const float screenWidthMeters = SCREEN_WIDTH / PIXELS_PER_METER;
        const float screenHeightMeters = SCREEN_HEIGHT / PIXELS_PER_METER;

        // Calculate spawn boundaries
        const float MIN_X = BOUNDARY_THICKNESS_METERS + BALL_RADIUS_METERS;
        const float MAX_X = screenWidthMeters - BOUNDARY_THICKNESS_METERS - BALL_RADIUS_METERS;
        const float MIN_Y = BOUNDARY_THICKNESS_METERS + BALL_RADIUS_METERS;
        const float MAX_Y = screenHeightMeters - BOUNDARY_THICKNESS_METERS - BALL_RADIUS_METERS;

        // Random position within safe spawn area (in meters)
        float randomX = MIN_X + static_cast<float>(rand()) / RAND_MAX * (MAX_X - MIN_X);
        float randomY = MIN_Y + static_cast<float>(rand()) / RAND_MAX * (MAX_Y - MIN_Y);

        auto ball = EntityManager::instance().addEntity("ball");
        ball->addComponent<CTransform>(Vec2(randomX, randomY), Vec2(1.0f, 1.0f), 0.0f);

        auto* physicsBody = ball->addComponent<CPhysicsBody2D>();
        physicsBody->initialize({randomX, randomY}, BodyType::Dynamic);

        auto* collider = ball->addComponent<CCollider2D>();
        collider->createCircle(BALL_RADIUS_METERS);
        collider->setRestitution(RESTITUTION);

        // Randomize initial velocity
        physicsBody->setLinearVelocity({getRandomVelocity().x, getRandomVelocity().y});
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

    void restart()
    {
        std::cout << "\n=== Restarting scenario ===" << std::endl;
        std::cout << "Ball count: " << m_ballAmount << std::endl;
        std::cout << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << std::endl;

        // Clear all entities
        EntityManager::instance().clear();

        // Reset physics world
        auto& physics = SBox2DPhysics::instance();
        physics.setGravity({0.0f, m_gravityEnabled ? GRAVITY_FORCE : 0.0f});

        // Recreate boundary colliders and balls
        createBoundaryColliders();
        createBalls();

        // Force EntityManager to process pending entities
        EntityManager::instance().update(0.0f);

        std::cout << "=== Restart complete ===" << std::endl;
    }

    void drawVector(const Vec2& startMeters, const Vec2& vectorMeters, const sf::Color& color, float scale = 1.0f)
    {
        // Convert physics positions to screen coordinates
        sf::Vector2f startPixels = metersToPixels(startMeters);

        // For vectors, we only need to scale and flip Y direction
        Vec2 scaledVector = vectorMeters * scale;
        sf::Vector2f vectorPixels(scaledVector.x * PIXELS_PER_METER, -scaledVector.y * PIXELS_PER_METER); // Negative Y for screen
        sf::Vector2f endPixels = startPixels + vectorPixels;

        // Draw line using VertexArray
        sf::VertexArray line(sf::Lines, 2);
        line[0].position = startPixels;
        line[0].color    = color;
        line[1].position = endPixels;
        line[1].color    = color;

        m_window.draw(line);
    }

    void update(float dt)
    {
        // Update Box2D physics
        SBox2DPhysics::instance().update(dt);

        // Update EntityManager
        EntityManager::instance().update(dt);
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
                if (!boundaryCollider->hasComponent<CTransform>() || !boundaryCollider->hasComponent<CCollider2D>())
                    continue;

                auto* transform = boundaryCollider->getComponent<CTransform>();
                auto* collider  = boundaryCollider->getComponent<CCollider2D>();

                Vec2 posMeters  = transform->getPosition();
                sf::Vector2f posPixels = metersToPixels(posMeters);

                float halfWidth = collider->getBoxHalfWidth() * PIXELS_PER_METER;
                float halfHeight = collider->getBoxHalfHeight() * PIXELS_PER_METER;

                sf::RectangleShape shape(sf::Vector2f(halfWidth * 2, halfHeight * 2));
                shape.setOrigin(halfWidth, halfHeight);
                shape.setPosition(posPixels);
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
            if (!ball->hasComponent<CTransform>() || !ball->hasComponent<CCollider2D>())
                continue;

            auto* transform = ball->getComponent<CTransform>();
            auto* collider  = ball->getComponent<CCollider2D>();

            Vec2 posMeters  = transform->getPosition();
            sf::Vector2f posPixels = metersToPixels(posMeters);
            float radiusPixels = collider->getCircleRadius() * PIXELS_PER_METER;

            sf::CircleShape ballShape(radiusPixels);
            ballShape.setOrigin(radiusPixels, radiusPixels);
            ballShape.setPosition(posPixels);

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

        // Draw velocity vectors
        if (m_showVectors)
        {
            for (auto& ball : balls)
            {
                if (!ball->hasComponent<CTransform>() || !ball->hasComponent<CPhysicsBody2D>())
                    continue;

                auto* transform = ball->getComponent<CTransform>();
                auto* physicsBody = ball->getComponent<CPhysicsBody2D>();

                Vec2 posMeters = transform->getPosition();

                // Draw velocity vector (yellow)
                b2Vec2 velocity = physicsBody->getLinearVelocity();
                Vec2 velocityMeters(velocity.x, velocity.y);
                if (velocityMeters.length() > 0.01f)  // Only draw if velocity is non-negligible
                {
                    drawVector(posMeters, velocityMeters, sf::Color::Yellow, 0.5f);
                }
            }
        }

        // Draw UI text showing current status
        if (m_fontLoaded)
        {
            std::ostringstream oss;
            oss << "Box2D Physics (1 unit = 1 meter, Y-up)\n";
            oss << "Ball Count: " << m_ballAmount << " (Use Left/Right to add/remove)\n";
            oss << "Gravity: " << (m_gravityEnabled ? "ON" : "OFF") << " (Press G to toggle)\n";
            oss << "Colliders: " << (m_showColliders ? "ON" : "OFF") << " (Press C to toggle)\n";
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
