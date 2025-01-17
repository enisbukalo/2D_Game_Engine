#ifndef CGRAVITY_H
#define CGRAVITY_H

#include "Component.h"
#include "Vec2.h"

struct CGravity : public Component
{
    Vec2 force = Vec2(0.0f, -9.81f);

    void update(float deltaTime) override;

    std::string getType() const override
    {
        return "Gravity";
    }

    json serialize() const override
    {
        json j     = Component::serialize();
        j["force"] = {{"x", force.x}, {"y", force.y}};
        return j;
    }

    void deserialize(const json& data) override
    {
        if (data.contains("force"))
        {
            force.x = data["force"]["x"];
            force.y = data["force"]["y"];
        }
    }
};

#endif  // CGRAVITY_H