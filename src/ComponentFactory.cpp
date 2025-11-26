#include "ComponentFactory.h"
#include "CCollider2D.h"
#include "CName.h"
#include "CPhysicsBody2D.h"
#include "CTransform.h"
#include "CInputController.h"

ComponentFactory::ComponentFactory()
{
    registerBuiltInComponents();
}

ComponentFactory& ComponentFactory::instance()
{
    static ComponentFactory instance;
    return instance;
}

Component* ComponentFactory::createComponent(const std::string& type)
{
    auto it = m_creators.find(type);
    if (it != m_creators.end())
    {
        return it->second();
    }
    return nullptr;
}

void ComponentFactory::registerBuiltInComponents()
{
    registerComponent<CTransform>("Transform");
    registerComponent<CName>("Name");

    // Box2D physics components
    registerComponent<CPhysicsBody2D>("CPhysicsBody2D");
    registerComponent<CPhysicsBody2D>("PhysicsBody2D");  // Alias for easier JSON usage
    registerComponent<CCollider2D>("CCollider2D");
    registerComponent<CCollider2D>("Collider2D");  // Alias for easier JSON usage
    registerComponent<CInputController>("CInputController");
    registerComponent<CInputController>("InputController");  // Alias for easier JSON usage
}