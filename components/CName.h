#ifndef CNAME_H
#define CNAME_H

#include "Component.h"

struct CName : public Component
{
#pragma region Variables
    std::string name;
#pragma endregion

#pragma region Constructors
    explicit CName(const std::string& n = "") : name(n) {}
#pragma endregion

#pragma region Override Methods
    std::string getType() const override;
    json        serialize() const override;
    void        deserialize(const json& data) override;
#pragma endregion
};

#endif  // CNAME_H