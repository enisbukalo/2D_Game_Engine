#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <vector>

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

class EventManager
{
public:
    EventManager(sf::RenderWindow* window, std::vector<b2BodyId*> bodiesToMove, b2WorldDef* worldDef);
    ~EventManager();

    void handleEvent(sf::Event event);
private:
    bool gravityOn;

    std::vector<b2BodyId*> bodiesToMove;
    sf::RenderWindow* mainWindow;
    b2WorldDef* worldDef;
};

#endif // EVENTMANAGER_H