#ifndef COMPONENT_H
#define COMPONENT_H

#include <Entity.h>
#include <SSerialization.h>
#include <string>

namespace Components
{

/**
 * @brief Lightweight data holder for components
 *
 * Components are now plain data; systems own all behavior.
 */
struct Component
{
    Component();
    ~Component() = default;

    bool        isActive() const;
    Entity      getOwner() const;
    void        setOwner(Entity owner);
    void        setActive(bool active);
    const std::string& getGuid() const;
    void              setGuid(const std::string& guid);

    Entity      m_owner;          ///< Entity ID that owns this component
    bool        m_active = true;  ///< Flag indicating if the component is active
    std::string m_guid;           ///< Unique identifier for this component
};

}  // namespace Components

#endif  // COMPONENT_H