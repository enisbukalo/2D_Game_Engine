#ifndef COMPONENT_H
#define COMPONENT_H

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class Entity;  // Forward declaration

class Component
{
public:
#pragma region Constructors
    Component()          = default;
    virtual ~Component() = default;
#pragma endregion

#pragma region Virtual Methods
    virtual void        init() {}
    virtual void        update(float deltaTime) {}
    virtual json        serialize() const;
    virtual void        deserialize(const json& data) {}
    virtual std::string getType() const = 0;
#pragma endregion

#pragma region Variables
    Entity* owner = nullptr;
#pragma endregion

private:
#pragma region Variables
    bool m_active = true;
#pragma endregion

public:
#pragma region Methods
    bool isActive() const;
    void setActive(bool active);
#pragma endregion
};

#endif  // COMPONENT_H