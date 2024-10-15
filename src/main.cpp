#include <iostream>

#include <SFML/Graphics.hpp>

unsigned int width = 1920;
unsigned int height = 1080;

sf::Vector2u windowSize = sf::Vector2u(width, height);

bool paused = false;

int main()
{
    auto window = sf::RenderWindow({1920u, 1080u}, "2D Space Game");
    window.setFramerateLimit(144);

    while (window.isOpen())
    {
        for (auto event = sf::Event(); window.pollEvent(event);)
        {
            // Close Window Event
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        if (!window.hasFocus())
        {
            paused = true;
            std::cout << "Paused" << std::endl;
        }
        else {
            paused = false;
            width = window.getSize().x;
            height = window.getSize().y;
            std::cout << "Width: " << width << "px" << std::endl;
            std::cout << "Height: " << height << "px"  << std::endl;
        }

        window.clear();
        window.display();
    }
}
