#ifndef WORLD_H
#define WORLD_H

#include <typeindex>

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
    void   queueDestroy(Entity e) { m_registry.queueDestroy(e); }
    void   processDestroyQueue() { m_registry.processDestroyQueue(); }
    size_t pendingDestroyCount() const { return m_registry.pendingDestroyCount(); }
    bool   isAlive(Entity e) const { return m_registry.isAlive(e); }

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

    template <typename... Components, typename Func>
    void view(Func&& fn)
    {
        m_registry.view<Components...>(std::forward<Func>(fn));
    }

    template <typename... Components, typename Func>
    void view(Func&& fn) const
    {
        m_registry.view<Components...>(std::forward<Func>(fn));
    }

    template <typename... Components, typename Func>
    void viewSorted(Func&& fn)
    {
        m_registry.viewSorted<Components...>(std::forward<Func>(fn));
    }

    template <typename... Components, typename Func, typename Compare>
    void viewSorted(Func&& fn, Compare&& compare)
    {
        m_registry.viewSorted<Components...>(std::forward<Func>(fn), std::forward<Compare>(compare));
    }

    template <typename... Components, typename Func>
    void viewSorted(Func&& fn) const
    {
        m_registry.viewSorted<Components...>(std::forward<Func>(fn));
    }

    template <typename... Components, typename Func, typename Compare>
    void viewSorted(Func&& fn, Compare&& compare) const
    {
        m_registry.viewSorted<Components...>(std::forward<Func>(fn), std::forward<Compare>(compare));
    }

    template <typename A, typename B, typename Func>
    void view2(Func&& fn)
    {
        m_registry.view2<A, B>(std::forward<Func>(fn));
    }

    template <typename A, typename B, typename Func>
    void view2(Func&& fn) const
    {
        m_registry.view2<A, B>(std::forward<Func>(fn));
    }

    template <typename A, typename B, typename C, typename Func>
    void view3(Func&& fn)
    {
        m_registry.view3<A, B, C>(std::forward<Func>(fn));
    }

    template <typename A, typename B, typename C, typename Func>
    void view3(Func&& fn) const
    {
        m_registry.view3<A, B, C>(std::forward<Func>(fn));
    }

    const std::vector<Entity>& getEntities() const { return m_registry.getEntities(); }
    const std::vector<std::type_index>& getComposition(Entity e) const { return m_registry.getComposition(e); }

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
