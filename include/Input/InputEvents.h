#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <string>
#include "Input/KeyCode.h"

enum class InputEventType
{
    KeyPressed,
    KeyReleased,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseWheel,
    TextEntered,
    WindowClosed,
    WindowResized,
    Action
};

struct KeyEvent
{
    KeyCode key;
    bool    alt;
    bool    ctrl;
    bool    shift;
    bool    system;
    bool    repeat;
};

struct MouseEvent
{
    sf::Mouse::Button button;
    sf::Vector2i      position;
    unsigned int      clickCount = 1;
};

struct MouseMoveEvent
{
    sf::Vector2i position;
};

struct WheelEvent
{
    float        delta;
    sf::Vector2i position;
};

struct TextEvent
{
    uint32_t unicode;
};

struct WindowEvent
{
    unsigned int width;
    unsigned int height;
};

enum class ActionState
{
    None = 0,
    Pressed,
    Held,
    Released
};

struct ActionEvent
{
    std::string actionName;
    ActionState state = ActionState::None;
};

struct InputEvent
{
    InputEventType type;
    // Instead of union, use separate fields
    // Only one will be valid based on 'type'
    KeyEvent       key{};
    MouseEvent     mouse{};
    MouseMoveEvent mouseMove{};
    WheelEvent     wheel{};
    TextEvent      text{};
    WindowEvent    window{};
    ActionEvent    action{};
};
