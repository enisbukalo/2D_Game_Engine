#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Component.h"

/**
 * @brief Factory class for creating components by type string
 *
 * @description
 * ComponentFactory implements the factory pattern for component creation,
 * allowing components to be created by their type string. This is particularly
 * useful for serialization/deserialization and dynamic component creation.
 * The factory is implemented as a singleton to ensure a single point of
 * component registration and creation.
 */
class ComponentFactory
{
public:
    /**
     * @brief Gets the singleton instance of the factory
     * @return Reference to the singleton instance
     */
    static ComponentFactory& instance();

    /**
     * @brief Registers a component type with the factory
     * @tparam T The component type to register
     * @param type String identifier for the component type
     */
    template <typename T>
    void registerComponent(const std::string& type)
    {
        m_creators[type] = []() { return new T(); };
    };
    /**
     * @brief Creates a component instance by type string
     * @param type String identifier of the component type
     * @return Pointer to the newly created component, or nullptr if type not found
     */
    Component* createComponent(const std::string& type);

private:
    /** @brief Private constructor for singleton pattern */
    ComponentFactory();

    /** @brief Default destructor */
    ~ComponentFactory() = default;

    /** @brief Deleted copy constructor */
    ComponentFactory(const ComponentFactory&) = delete;

    /** @brief Deleted assignment operator */
    ComponentFactory& operator=(const ComponentFactory&) = delete;

    /** @brief Registers all built-in component types */
    void registerBuiltInComponents();

    std::unordered_map<std::string, std::function<Component*()>> m_creators;  ///< Map of component creators by type string
};

#endif  // COMPONENTFACTORY_H