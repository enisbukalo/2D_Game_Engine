#include <iostream>
#include <vector>

#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>

#include "Entity.h"
#include "GameEngine.h"
#include "Vec2.h"

#pragma region Variables

unsigned int width      = 1920;
unsigned int height     = 1080;
sf::Vector2u windowSize = sf::Vector2u(width, height);

// Gravity in m/s^2 {x, y}.
b2Vec2 gravity = b2Vec2{0.0f, 9.81f};
// Limit to 60 fps.
const float timeStep = 1.0f / 60.0f;
// Less = Performance. More = Accuracy.
const int subStepCount = 4;

// const float playerPixelHeight = 100.0f;
// const float playerPixelWidth = 100.0f;
// const float groundPixelHeight = 10.0f;
// const float groundPixelWidth = 1000.0f;
// const float PPM = 32.0f;

// bool paused = false;

// std::vector<sf::Shape *> shapes;
// std::vector<sf::VertexArray *> lines;
#pragma endregion

// const float pixelToMeter(float pixels) { return pixels / PPM; }
// const float meterToPixel(float meters) { return meters * PPM; }

// int main()
// {
// #pragma region Window Settings
//   auto window = sf::RenderWindow({1920u, 1080u}, "2D Space Game");
//   window.setFramerateLimit(144);

//   b2WorldDef worldDef = b2DefaultWorldDef();
//   worldDef.gravity = gravity;
//   b2WorldId worldId = b2CreateWorld(&worldDef);
// #pragma endregion

// #pragma region GroundBox
//   b2BodyDef groundBodyDef = b2DefaultBodyDef();
//   groundBodyDef.position = (b2Vec2){pixelToMeter(groundPixelWidth) / 2.0f, pixelToMeter(groundPixelHeight) / 2.0f};
//   b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);
//   b2Polygon groundBox = b2MakeBox(groundPixelWidth, pixelToMeter(groundPixelHeight));
//   b2ShapeDef groundShapeDef = b2DefaultShapeDef();
//   b2ShapeId groundShapeId = b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
// #pragma endregion

// #pragma region PlayerBox
//   b2BodyDef playerBodyDef = b2DefaultBodyDef();
//   playerBodyDef.type = b2_dynamicBody;
//   playerBodyDef.position = (b2Vec2){pixelToMeter(playerPixelWidth) / 2.0f, pixelToMeter(playerPixelHeight) / 2.0f};
//   b2BodyId playerBodyId = b2CreateBody(worldId, &playerBodyDef);
//   b2Polygon playerBox = b2MakeBox(pixelToMeter(playerPixelWidth), pixelToMeter(playerPixelHeight));
//   b2ShapeDef playerShapeDef = b2DefaultShapeDef();
//   playerShapeDef.density = 0.5f;
//   playerShapeDef.friction = 0.3f;
//   b2ShapeId playerShapeId = b2CreatePolygonShape(playerBodyId, &playerShapeDef, &playerBox);
// #pragma endregion

// #pragma region Renders
//   sf::RectangleShape playerSprite(sf::Vector2f(playerPixelWidth, playerPixelHeight));
//   sf::RectangleShape groundSprite(sf::Vector2f(groundPixelWidth, groundPixelHeight));

//   playerSprite.setOrigin(playerPixelWidth / 2.0f, playerPixelHeight / 2.0f);
//   groundSprite.setOrigin(groundPixelWidth / 2.0f, groundPixelHeight / 2.0f);

//   playerSprite.setFillColor(sf::Color(100, 250, 50));
//   groundSprite.setFillColor(sf::Color(250, 250, 0));

//   shapes.push_back(&playerSprite);
//   shapes.push_back(&groundSprite);
// #pragma endregion

//   sf::VertexArray lines(sf::Lines, 4);
//   lines[0].color = sf::Color::Red;
//   lines[1].color = sf::Color::Red;
//   lines[2].color = sf::Color::Green;
//   lines[3].color = sf::Color::Green;

//   EventManager eventManager = EventManager(&window, {&playerBodyId}, &worldDef);

//   while (window.isOpen()) {
//     for (auto event = sf::Event(); window.pollEvent(event);) {
//       eventManager.handleEvent(event);
//     }

//     if (!window.hasFocus()) {
//       paused = true;
//       // std::cout << "Paused" << std::endl;
//     }
//     else {
//       paused = false;
//       width = window.getSize().x;
//       height = window.getSize().y;
//       // std::cout << "Width: " << width << "px" << std::endl;
//       // std::cout << "Height: " << height << "px"  << std::endl;
//       b2World_Step(worldId, timeStep, subStepCount);

//       b2Vec2 playerPhysicsPos = b2Body_GetPosition(b2Shape_GetBody(playerShapeId));
//       b2Vec2 groundPhysicsPos2 = b2Body_GetPosition(b2Shape_GetBody(groundShapeId));
//       b2Rot playerPhysicsRot = b2Body_GetRotation(b2Shape_GetBody(playerShapeId));

//       std::cout << "Position: " << playerPhysicsPos.x << ", " << playerPhysicsPos.y << std::endl;
//       std::cout << "Rotation: " << b2Rot_GetAngle(playerPhysicsRot) << std::endl;
//       groundSprite.setPosition(groundPhysicsPos2.x, groundPhysicsPos2.y);
//       playerSprite.setPosition(playerPhysicsPos.x, playerPhysicsPos.y);
//       playerSprite.setRotation(b2Rot_GetAngle(playerPhysicsRot));

//       lines[0].position = sf::Vector2f(playerPhysicsPos.x, playerPhysicsPos.y);
//       lines[1].position = sf::Vector2f(groundPhysicsPos2.x, groundPhysicsPos2.y);
//       lines[2].position = sf::Vector2f(playerSprite.getPosition().x, playerSprite.getPosition().y);
//       lines[3].position = sf::Vector2f(groundSprite.getPosition().x, groundSprite.getPosition().y);

//       window.clear();
//       for (sf::Shape *shape : shapes) {
//         window.draw(*shape);
//       }

//       window.draw(lines);
//       window.display();
//     }
//   }

//   b2DestroyWorld(worldId);
// }

int main(int argc, char *argv[])
{
    sf::VideoMode    videoMode = sf::VideoMode(windowSize.x, windowSize.y);
    sf::RenderWindow window    = sf::RenderWindow(videoMode, "2D Space Game");
    GameEngine       game      = GameEngine(&window, gravity, subStepCount, timeStep);

    Vec2 v1 = Vec2(150.0f, 200.0f);
    Vec2 v2 = Vec2(25.0f, 15.0f);
    Vec2 v3 = Vec2(50.0f, 50.0f);

    std::cout << "Distance: V1 To V2: " << v1.distance(v2) << std::endl;

    while (game.is_running())
    {
        game.readInputs();
        game.update();
        game.render();
    }

    return 0;
}
