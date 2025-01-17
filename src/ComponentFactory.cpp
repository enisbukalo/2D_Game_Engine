#include "../include/ComponentFactory.h"
#include "../components/CGravity.h"
#include "../components/CName.h"
#include "../components/CTransform.h"

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
    registerComponent<CName>("Name");
}