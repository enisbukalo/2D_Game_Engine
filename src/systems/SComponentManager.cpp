#include "SComponentManager.h"
#include <algorithm>
#include "CPhysicsBody2D.h"
#include "Component.h"

namespace Systems
{

SComponentManager& SComponentManager::instance()
{
    static SComponentManager s_instance;
    return s_instance;
}

void SComponentManager::registerComponent(::Components::Component* comp)
{
    if (!comp)
        return;
    std::lock_guard<std::mutex> guard(m_mutex);
    // If already present in active or inactive, skip
    if (std::find(m_activeComponents.begin(), m_activeComponents.end(), comp) != m_activeComponents.end())
        return;
    if (std::find(m_inactiveComponents.begin(), m_inactiveComponents.end(), comp) != m_inactiveComponents.end())
        return;

    if (comp->isActive())
    {
        m_activeComponents.push_back(comp);
    }
    else
    {
        m_inactiveComponents.push_back(comp);
    }

    // Check if this is a physics component
    if (auto physics = dynamic_cast<::Components::CPhysicsBody2D*>(comp))
    {
        if (std::find(m_physicsComponents.begin(), m_physicsComponents.end(), physics) == m_physicsComponents.end())
            m_physicsComponents.push_back(physics);
    }
}

void SComponentManager::unregisterComponent(::Components::Component* comp)
{
    if (!comp)
        return;

    std::lock_guard<std::mutex> guard(m_mutex);
    auto                        activeIt = std::find(m_activeComponents.begin(), m_activeComponents.end(), comp);
    if (activeIt != m_activeComponents.end())
        m_activeComponents.erase(activeIt);

    auto inactiveIt = std::find(m_inactiveComponents.begin(), m_inactiveComponents.end(), comp);
    if (inactiveIt != m_inactiveComponents.end())
        m_inactiveComponents.erase(inactiveIt);

    if (auto physics = dynamic_cast<::Components::CPhysicsBody2D*>(comp))
    {
        auto pIt = std::find(m_physicsComponents.begin(), m_physicsComponents.end(), physics);
        if (pIt != m_physicsComponents.end())
            m_physicsComponents.erase(pIt);
    }
}

void SComponentManager::setActive(::Components::Component* comp, bool active)
{
    if (!comp)
        return;

    std::lock_guard<std::mutex> guard(m_mutex);

    // If active, ensure in active list, remove from inactive list
    if (active)
    {
        auto it = std::find(m_activeComponents.begin(), m_activeComponents.end(), comp);
        if (it == m_activeComponents.end())
        {
            m_activeComponents.push_back(comp);
        }

        auto inactiveIt = std::find(m_inactiveComponents.begin(), m_inactiveComponents.end(), comp);
        if (inactiveIt != m_inactiveComponents.end())
            m_inactiveComponents.erase(inactiveIt);
    }
    else
    {
        auto inactiveIt = std::find(m_inactiveComponents.begin(), m_inactiveComponents.end(), comp);
        if (inactiveIt == m_inactiveComponents.end())
        {
            m_inactiveComponents.push_back(comp);
        }

        auto activeIt = std::find(m_activeComponents.begin(), m_activeComponents.end(), comp);
        if (activeIt != m_activeComponents.end())
            m_activeComponents.erase(activeIt);
    }
}

const std::vector<::Components::Component*>& SComponentManager::getActiveComponents() const
{
    return m_activeComponents;
}

const std::vector<::Components::CPhysicsBody2D*>& SComponentManager::getPhysicsComponents() const
{
    return m_physicsComponents;
}

void SComponentManager::updateAll(float dt)
{
    std::vector<::Components::Component*> snapshot;
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        snapshot = m_activeComponents;  // shallow copy for safe iteration
    }

    for (auto comp : snapshot)
    {
        if (!comp)
            continue;
        if (comp->isActive())
        {
            comp->update(dt);
        }
    }
}

}  // namespace Systems
