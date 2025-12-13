#pragma once

#include <string>
#include <vector>
#include "KeyCode.h"
#include "MouseButton.h"

enum class ActionTrigger
{
    Pressed,
    Held,
    Released
};

struct ActionBinding
{
    std::vector<KeyCode>     keys;
    std::vector<MouseButton> mouseButtons;
    ActionTrigger            trigger     = ActionTrigger::Pressed;
    bool                     allowRepeat = false;
};
