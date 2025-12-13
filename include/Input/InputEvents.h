#pragma once

#include <string>
#include "KeyCode.h"
#include "MouseButton.h"
#include "Vec2i.h"

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
    MouseButton  button;
    Vec2i        position;
    unsigned int clickCount = 1;
};

struct MouseMoveEvent
{
    Vec2i position;
};

struct WheelEvent
{
    float delta;
    Vec2i position;
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
