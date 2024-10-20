#include <iostream>

#include "EventManager.h"

EventManager::EventManager(sf::RenderWindow* window, std::vector<b2BodyId*> bodyIds, b2WorldDef* worldDef) : mainWindow(window), bodiesToMove(bodyIds), worldDef(worldDef)
{
    gravityOn = worldDef->gravity.y <= 0.0f ? false : true;
}

EventManager::~EventManager() {}

void EventManager::handleEvent(sf::Event event)
{
    // Close Window Event
    if (event.type == sf::Event::Closed)
    {
        mainWindow->close();
    }

    if (event.type == sf::Event::KeyPressed)
    {
        float forceX = 0.0f;
        float forceY = 0.0f;
        float torqueForce = 0.0f;

        switch (event.key.code)
        {
            case sf::Keyboard::G:
                gravityOn = !gravityOn;
                if (gravityOn)
                {
                    worldDef->gravity = (b2Vec2){0.0f, 9.81f};
                }
                else
                {
                    worldDef->gravity = (b2Vec2){0.0f, 0.0f};
                }
                std::cout << "Gravity is " << (gravityOn ? "ON" : "OFF") << std::endl;
                break;
            case sf::Keyboard::D:
                forceX = 10000.0f;
                break;
            case sf::Keyboard::A:
                forceX = -10000.0f;
                break;
            case sf::Keyboard::W:
                forceY = -10000.0f;
                break;
            case sf::Keyboard::E:
                torqueForce = 1000000.0f;
                break;
            case sf::Keyboard::Q:
                torqueForce = -1000000.0f;
                break;
            default:
                break;
        }
        for (b2BodyId* bodyId : bodiesToMove)
        {
            b2Body_ApplyForceToCenter(*bodyId, (b2Vec2){forceX, forceY}, true);
            b2Body_ApplyTorque(*bodyId, torqueForce, true);
        }
    }
}