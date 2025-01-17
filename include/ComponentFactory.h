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
#pragma region Singleton
    static ComponentFactory& instance();
#pragma endregion

#pragma region Templates
    template <typename T>
    void registerComponent(const std::string& type)
    {
        m_creators[type] = []() { return new T(); };
    }
#pragma endregion

#pragma region Methods
    Component* createComponent(const std::string& type);
#pragma endregion

private:
#pragma region Constructors
    ComponentFactory();
    ~ComponentFactory()                                  = default;
    ComponentFactory(const ComponentFactory&)            = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;
#pragma endregion

#pragma region Methods
    void registerBuiltInComponents();
#pragma endregion

#pragma region Variables
    std::unordered_map<std::string, std::function<Component*()>> m_creators;
#pragma endregion
};

#endif  // COMPONENTFACTORY_H