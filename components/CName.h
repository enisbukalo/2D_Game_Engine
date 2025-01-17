#ifndef CNAME_H
#define CNAME_H

#include "Component.h"

struct CName : public Component
{
    std::string name;

    explicit CName(const std::string& n = "") : name(n) {}

    std::string getType() const override;
    json        serialize() const override;
    void        deserialize(const json& data) override;
};

#endif  // CNAME_H