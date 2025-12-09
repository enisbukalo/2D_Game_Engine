#ifndef SCOMPONENTMANAGER_H
#define SCOMPONENTMANAGER_H

#include <mutex>
#include <vector>

namespace Components
{
class Component;
class CPhysicsBody2D;
}  // namespace Components

namespace Systems
{
class SComponentManager
{
public:
    static SComponentManager& instance();

    void registerComponent(::Components::Component* comp);
    void unregisterComponent(::Components::Component* comp);
    void setActive(::Components::Component* comp, bool active);

    const std::vector<::Components::Component*>&      getActiveComponents() const;
    const std::vector<::Components::CPhysicsBody2D*>& getPhysicsComponents() const;

    // Update all active components
    void updateAll(float dt);

private:
    SComponentManager()                                    = default;
    ~SComponentManager()                                   = default;
    SComponentManager(const SComponentManager&)            = delete;
    SComponentManager& operator=(const SComponentManager&) = delete;

    std::vector<::Components::Component*>      m_activeComponents;
    std::vector<::Components::Component*>      m_inactiveComponents;
    std::vector<::Components::CPhysicsBody2D*> m_physicsComponents;
    std::mutex                                 m_mutex;
};

}  // namespace Systems

#endif  // SCOMPONENTMANAGER_H
