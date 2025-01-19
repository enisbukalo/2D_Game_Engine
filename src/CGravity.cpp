#include "CGravity.h"

void CGravity::update(float deltaTime)
{
    // Implementation will be added when physics system is implemented
}

std::string CGravity::getType() const
{
    return "Gravity";
}

json CGravity::serialize() const
{
    json j     = Component::serialize();
    j["force"] = {{"x", force.x}, {"y", force.y}};
    return j;
}

void CGravity::deserialize(const json& data)
{
    if (data.contains("force"))
    {
        force.x = data["force"]["x"];
        force.y = data["force"]["y"];
    }
}