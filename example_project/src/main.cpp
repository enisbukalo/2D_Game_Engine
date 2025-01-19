#include <CName.h>
#include <CTransform.h>
#include <EntityManager.h>
#include <GameEngine.h>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

int main()
{
    // Create SFML window
    sf::RenderWindow window(sf::VideoMode(800, 600), "GameEngine Example");

    // Create game engine instance with parameters
    GameEngine engine(&window, sf::Vector2f(0.0f, 9.81f), 8, 1.0f / 60.0f);

    // Create entity manager
    EntityManager entityManager;

    // Create an entity
    auto entity = entityManager.addEntity("example");

    // Add components to the entity
    entity->addComponent<CTransform>(Vec2(100.0f, 100.0f), Vec2(1.0f, 1.0f), 0.0f);
    entity->addComponent<CName>("Example Entity");

    // Setup random number generation
    std::random_device                    rd;
    std::mt19937                          gen(rd());
    std::uniform_real_distribution<float> pos_dist(0.0f, 800.0f);     // Random position between 0 and 800
    std::uniform_real_distribution<float> vel_dist(-100.0f, 100.0f);  // Random velocity between -100 and 100
    std::uniform_real_distribution<float> scale_dist(0.5f, 2.0f);     // Random scale between 0.5 and 2
    std::uniform_real_distribution<float> rot_dist(0.0f, 360.0f);     // Random rotation between 0 and 360

    // Print initial entity information
    auto* transform = entity->getComponent<CTransform>();
    auto* name      = entity->getComponent<CName>();

    if (transform && name)
    {
        std::cout << "Created entity: " << name->getName() << std::endl;
    }

    // Game loop
    while (engine.is_running())
    {
        engine.readInputs();

        // Randomly update transform components
        if (transform)
        {
            transform->setVelocity(Vec2(vel_dist(gen), vel_dist(gen)));
            transform->setPosition(Vec2(pos_dist(gen), pos_dist(gen)));
            transform->setScale(Vec2(scale_dist(gen), scale_dist(gen)));
            transform->setRotation(rot_dist(gen));

            // Print current values
            Vec2  vel   = transform->getVelocity();
            Vec2  pos   = transform->getPosition();
            Vec2  scale = transform->getScale();
            float rot   = transform->getRotation();

            std::cout << "\nFrame Update:" << std::endl;
            std::cout << "Velocity: (" << vel.x << ", " << vel.y << ")" << std::endl;
            std::cout << "Position: (" << pos.x << ", " << pos.y << ")" << std::endl;
            std::cout << "Scale: (" << scale.x << ", " << scale.y << ")" << std::endl;
            std::cout << "Rotation: " << rot << std::endl;
        }

        engine.update();
        engine.render();

        // Sleep for 500ms (2 updates per second)
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}