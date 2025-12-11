#ifndef WORLD_H
#define WORLD_H

#include "Registry.h"

/**
 * @brief World is the orchestration point for entities and components
 *
 * Thin wrapper over Registry to make the public API explicit for callers.
 */
class World
{
public:
    World()  = default;
    ~World() = default;

    World(const World&)            = delete;
    World& operator=(const World&) = delete;

    Entity createEntity() { return m_registry.createEntity(); }
    void   destroyEntity(Entity e) { m_registry.destroy(e); }

    template <typename T, typename... Args>
    T& add(Entity e, Args&&... args)
    {
        return m_registry.add<T>(e, std::forward<Args>(args)...);
    }

    template <typename T>
    void remove(Entity e)
    {
        m_registry.remove<T>(e);
    }

    template <typename T>
    bool has(Entity e) const
    {
        return m_registry.has<T>(e);
    }

    template <typename T>
    T& get(Entity e)
    {
        return m_registry.get<T>(e);
    }

    template <typename T>
    const T& get(Entity e) const
    {
        return m_registry.get<T>(e);
    }

    template <typename T>
    T* tryGet(Entity e)
    {
        return m_registry.tryGet<T>(e);
    }

    template <typename T>
    const T* tryGet(Entity e) const
    {
        return m_registry.tryGet<T>(e);
    }

    template <typename T, typename Func>
    void each(Func&& fn)
    {
        m_registry.each<T>(std::forward<Func>(fn));
    }

    template <typename T, typename Func>
    void each(Func&& fn) const
    {
        m_registry.each<T>(std::forward<Func>(fn));
    }

    const std::vector<Entity>& getEntities() const { return m_registry.getEntities(); }

    void clear() { m_registry.clear(); }

    template <typename T>
    void registerTypeName(const std::string& typeName)
    {
        m_registry.registerTypeName<T>(typeName);
    }

    template <typename T>
    std::string getTypeName() const
    {
        return m_registry.getTypeName<T>();
    }

    std::type_index getTypeFromName(const std::string& name) const { return m_registry.getTypeFromName(name); }

    // Transitional escape hatch while refactoring systems
    Registry&       registry() { return m_registry; }
    const Registry& registry() const { return m_registry; }

private:
    Registry m_registry;
};

#endif  // WORLD_H
