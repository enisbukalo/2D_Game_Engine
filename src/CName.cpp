#include "CName.h"

std::string CName::getType() const
{
    return "Name";
}

json CName::serialize() const
{
    json j    = Component::serialize();
    j["name"] = name;
    return j;
}

void CName::deserialize(const json& data)
{
    if (data.contains("name"))
    {
        name = data["name"];
    }
}