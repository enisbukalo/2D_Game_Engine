#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Component.h"

class ComponentFactory
{
public:
    static ComponentFactory& instance();

    template <typename T>
    void registerComponent(const std::string& type)
    {
        m_creators[type] = []() { return new T(); };
    }

    Component* createComponent(const std::string& type);

private:
    ComponentFactory();
    ~ComponentFactory()                                  = default;
    ComponentFactory(const ComponentFactory&)            = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;

    void                                                         registerBuiltInComponents();
    std::unordered_map<std::string, std::function<Component*()>> m_creators;
};

#endif  // COMPONENTFACTORY_H