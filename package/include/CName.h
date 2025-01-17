#ifndef CNAME_H
#define CNAME_H

#include "Component.h"

struct CName : public Component
{
    std::string name;

    explicit CName(const std::string& n = "") : name(n) {}

    std::string getType() const override
    {
        return "Name";
    }

    json serialize() const override
    {
        json j    = Component::serialize();
        j["name"] = name;
        return j;
    }

    void deserialize(const json& data) override
    {
        if (data.contains("name"))
        {
            name = data["name"];
        }
    }
};

#endif  // CNAME_H