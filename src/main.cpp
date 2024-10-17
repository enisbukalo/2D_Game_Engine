#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <box2d/types.h>
// #include <imgui.h>
// #include <backends/imgui_impl_glfw.h>
// #include <backends/imgui_impl_opengl3.h>

unsigned int width = 1920;
unsigned int height = 1080;

sf::Vector2u windowSize = sf::Vector2u(width, height);

bool paused = false;

std::vector<sf::Shape*> shapes;

int main()
{
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, 9.81f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = (b2Vec2){0.0f, 900.0f};

    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

    b2Polygon groundBox = b2MakeBox(1000.0f, 10.0f);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){0.0f, 4.0f};
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    b2Polygon dynamicBox = b2MakeBox(100.0f, 100.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;

    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

    // Limit to 60 fps.
    float timeStep = 1.0f/60.0f;
    // Less = Performance. More = Accuracy.
    int subStepCount = 4;

    auto window = sf::RenderWindow({1920u, 1080u}, "2D Space Game");
    window.setFramerateLimit(144);

    sf::CircleShape shape(50);
    sf::RectangleShape shape2(sf::Vector2f(100, 100));
    sf::RectangleShape groundShape(sf::Vector2f(1000, 10));
    groundShape.setPosition(b2Body_GetPosition(groundId).x, b2Body_GetPosition(groundId).y);
    shape.setFillColor(sf::Color(100, 250, 50));

    shapes.push_back(&shape);
    shapes.push_back(&shape2);
    shapes.push_back(&groundShape);

    while (window.isOpen())
    {
        for (auto event = sf::Event(); window.pollEvent(event);)
        {
            // Close Window Event
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                    case sf::Keyboard::D:
                        b2Body_ApplyForceToCenter(bodyId, (b2Vec2){10000000.0f, 0.0f}, true);
                        break;
                    case sf::Keyboard::A:
                        b2Body_ApplyForceToCenter(bodyId, (b2Vec2){-10000000.0f, 0.0f}, true);
                        break;
                    case sf::Keyboard::W:
                        b2Body_ApplyForceToCenter(bodyId, (b2Vec2){0.0f, -10000000.0f}, true);
                        break;
                    default:
                        break;
                }
            }
        }

        if (!window.hasFocus())
        {
            paused = true;
            // std::cout << "Paused" << std::endl;
        }
        else {
            paused = false;
            width = window.getSize().x;
            height = window.getSize().y;
            // std::cout << "Width: " << width << "px" << std::endl;
            // std::cout << "Height: " << height << "px"  << std::endl;
            b2World_Step(worldId, timeStep, subStepCount);
            b2Vec2 position = b2Body_GetPosition(bodyId);
            b2Rot rotation = b2Body_GetRotation(bodyId);
            std::cout << "Position: " << position.x << ", " << position.y << std::endl;
            std::cout << "Rotation: " << b2Rot_GetAngle(rotation) << std::endl;
            shape2.setPosition(position.x, position.y);
            shape2.setRotation(b2Rot_GetAngle(rotation));

            window.clear();
            for (sf::Shape* shape : shapes)
            {
                window.draw(*shape);
            }
            window.display();
        }


    }

    b2DestroyWorld(worldId);
}
