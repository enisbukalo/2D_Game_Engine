#include "ComponentFactory.h"
#include "CBoxCollider.h"
#include "CCircleCollider.h"
#include "CGravity.h"
#include "CName.h"
#include "CRigidBody2D.h"
#include "CTransform.h"

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
    registerComponent<CGravity>("Gravity");
    registerComponent<CRigidBody2D>("RigidBody2D");
    registerComponent<CName>("Name");
    registerComponent<CCircleCollider>("CircleCollider");
    registerComponent<CBoxCollider>("BoxCollider");
}