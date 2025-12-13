#include "InputHelpers.h"

#include <Components.h>
#include <InputEvents.h>

namespace Example
{

bool isActionActive(const Components::CInputController& input, const std::string& action)
{
    auto it = input.actionStates.find(action);
    if (it == input.actionStates.end())
    {
        return false;
    }
    return it->second == ActionState::Pressed || it->second == ActionState::Held;
}

}  // namespace Example
