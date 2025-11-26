#pragma once

#include <SFML/Window/Mouse.hpp>
#include <string>
#include <vector>
#include "Input/KeyCode.h"

enum class ActionTrigger
{
    Pressed,
    Held,
    Released
};

struct ActionBinding
{
    std::vector<KeyCode>           keys;
    std::vector<sf::Mouse::Button> mouseButtons;
    ActionTrigger                  trigger     = ActionTrigger::Pressed;
    bool                           allowRepeat = false;
};
