#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <vector>

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

class EventManager
{
public:
    EventManager(sf::RenderWindow* window, std::vector<b2BodyId*> bodiesToMove);
    ~EventManager();

    void handleEvent(sf::Event event);
private:
    std::vector<b2BodyId*> bodiesToMove;
    sf::RenderWindow* mainWindow;
};

#endif // EVENTMANAGER_H