#include "CName.h"

namespace Components
{

const std::string& CName::getName() const
{
    return name;
}

void CName::setName(const std::string& name)
{
    this->name = name;
}

}  // namespace Components
