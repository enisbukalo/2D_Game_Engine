#include "CName.h"

std::string CName::getType() const
{
    return "Name";
}

void CName::serialize() const {}

void CName::deserialize() {}

std::string CName::getName() const
{
    return m_name;
}

void CName::setName(const std::string& name)
{
    m_name = name;
}
