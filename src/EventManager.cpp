#include "EventManager.h"

EventManager::EventManager(sf::RenderWindow* window, std::vector<b2BodyId*> bodyIds) : mainWindow(window), bodiesToMove(bodyIds) {}

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
            case sf::Keyboard::D:
                forceX = 10000000.0f;
                break;
            case sf::Keyboard::A:
                forceX = -10000000.0f;
                break;
            case sf::Keyboard::W:
                forceY = -10000000.0f;
                break;
            case sf::Keyboard::E:
                torqueForce = 1000000000.0f;
                break;
            case sf::Keyboard::Q:
                torqueForce = -1000000000.0f;
                break;
            default:
                break;
        }
        for (b2BodyId* bodyId : bodiesToMove)
        {
            b2Body_ApplyForce(*bodyId, (b2Vec2){forceX, forceY}, b2Body_GetLocalCenterOfMass(*bodyId), true);
            b2Body_ApplyTorque(*bodyId, torqueForce, true);
        }
    }
}