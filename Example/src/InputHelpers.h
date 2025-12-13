#pragma once

#include <string>

namespace Components
{
struct CInputController;
}

namespace Example
{

bool isActionActive(const Components::CInputController& input, const std::string& action);

}  // namespace Example
