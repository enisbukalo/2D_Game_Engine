#pragma once

#include <cstdint>

/**
 * @brief Engine-agnostic mouse button codes used by the Input manager
 */
enum class MouseButton : uint8_t
{
    Left = 0,
    Right,
    Middle,
    XButton1,
    XButton2,
    Unknown
};
