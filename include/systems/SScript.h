#pragma once

#include "System.h"

namespace Systems
{

/**
 * @brief Runs per-entity native scripts (behaviours) stored in Components::CNativeScript.
 */
class SScript : public System
{
public:
    void update(float deltaTime, World& world) override;
};

}  // namespace Systems
