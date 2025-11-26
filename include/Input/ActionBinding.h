#pragma once

#include "Input/KeyCode.h"
#include <SFML/Window/Mouse.hpp>
#include <vector>
#include <string>

enum class ActionTrigger
{
    Pressed,
    Held,
    Released
};

struct ActionBinding
{
    std::vector<KeyCode> keys;
    std::vector<sf::Mouse::Button> mouseButtons;
    ActionTrigger trigger = ActionTrigger::Pressed;
    bool allowRepeat = false;
};
