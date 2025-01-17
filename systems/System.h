#ifndef SYSTEM_H
#define SYSTEM_H

#include <vector>
#include "../components/CGravity.h"
#include "../components/CName.h"
#include "../components/CTransform.h"
#include "Component.h"
#include "Entity.h"

class System
{
public:
#pragma region Constructors
    System();
    ~System();
#pragma endregion

#pragma region Virtual Methods
    virtual void update() = 0;
#pragma endregion

private:
#pragma region Variables
    std::unordered_map<uint8_t, std::unique_ptr<Component>> m_components;
#pragma endregion
};
#endif  // SYSTEM_H