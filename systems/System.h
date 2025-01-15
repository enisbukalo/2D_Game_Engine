#ifndef SYSTEM_H
#define SYSTEM_H

#include <memory>
#include <unordered_map>

#include "Component.h"

class System
{
public:
    System();
    ~System();

    virtual void update() = 0;

private:
    std::unordered_map<uint8_t, std::unique_ptr<Component>> m_components;
};
#endif  // SYSTEM_H