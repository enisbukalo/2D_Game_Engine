#ifndef REGISTRY_H
#define REGISTRY_H

#include <Entity.h>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

/**
 * @brief Type-erased interface for component storage
 *
 * Allows Registry to manage multiple ComponentStore<T> instances
 * in a type-safe container using runtime polymorphism.
 */
class IComponentStore
{
public:
    virtual ~IComponentStore() = default;

    /**
     * @brief Removes component for the specified entity (if it exists)
     */
    virtual void remove(Entity entity) = 0;

    /**
     * @brief Checks if entity has this component type
     */
    virtual bool has(Entity entity) const = 0;

    /**
     * @brief Gets count of components in this store
     */
    virtual size_t size() const = 0;

    /**
     * @brief Gets the type name for serialization
     */
    virtual std::string getTypeName() const = 0;
};

/**
 * @brief Dense component storage with sparse entity->index mapping
 *
 * Components are stored in a tightly-packed vector for cache-friendly iteration.
 * A sparse unordered_map translates EntityId to the dense array index.
 * When removing, we use swap-and-pop to keep the dense array compact.
 *
 * @tparam T Component type
 */
template <typename T>
class ComponentStore : public IComponentStore
{
public:
    ComponentStore() = default;

    /**
     * @brief Adds or replaces a component for an entity
     * @param entity Entity to add component to
     * @param args Arguments forwarded to component constructor
     * @return Reference to the created/replaced component
     */
    template <typename... Args>
    T& add(Entity entity, Args&&... args)
    {
        // Check if component already exists
        auto it = m_sparse.find(entity);
        if (it != m_sparse.end())
        {
            // Replace existing component
            m_dense[it->second] = T(std::forward<Args>(args)...);
            return m_dense[it->second];
        }

        // Add new component
        size_t index = m_dense.size();
        m_dense.emplace_back(std::forward<Args>(args)...);
        m_entities.push_back(entity);
        m_sparse[entity] = index;
        return m_dense[index];
    }

    /**
     * @brief Removes component for an entity using swap-and-pop
     * @param entity Entity to remove component from
     */
    void remove(Entity entity) override
    {
        auto it = m_sparse.find(entity);
        if (it == m_sparse.end())
        {
            return;  // Component doesn't exist
        }

        size_t indexToRemove = it->second;
        size_t lastIndex     = m_dense.size() - 1;

        // Swap with last element if not already last
        if (indexToRemove != lastIndex)
        {
            m_dense[indexToRemove]    = std::move(m_dense[lastIndex]);
            m_entities[indexToRemove] = m_entities[lastIndex];

            // Update sparse map for the swapped entity
            m_sparse[m_entities[indexToRemove]] = indexToRemove;
        }

        // Remove last element
        m_dense.pop_back();
        m_entities.pop_back();
        m_sparse.erase(it);
    }

    /**
     * @brief Checks if entity has this component
     */
    bool has(Entity entity) const override
    {
        return m_sparse.find(entity) != m_sparse.end();
    }

    /**
     * @brief Gets component for entity (asserts if not present)
     */
    T& get(Entity entity)
    {
        auto it = m_sparse.find(entity);
        assert(it != m_sparse.end() && "Entity does not have this component");
        return m_dense[it->second];
    }

    /**
     * @brief Gets component for entity (const version)
     */
    const T& get(Entity entity) const
    {
        auto it = m_sparse.find(entity);
        assert(it != m_sparse.end() && "Entity does not have this component");
        return m_dense[it->second];
    }

    /**
     * @brief Tries to get component, returns nullptr if not present
     */
    T* tryGet(Entity entity)
    {
        auto it = m_sparse.find(entity);
        return (it != m_sparse.end()) ? &m_dense[it->second] : nullptr;
    }

    /**
     * @brief Tries to get component (const version)
     */
    const T* tryGet(Entity entity) const
    {
        auto it = m_sparse.find(entity);
        return (it != m_sparse.end()) ? &m_dense[it->second] : nullptr;
    }

    /**
     * @brief Iterates over all components, calling fn(EntityId, T&) for each
     */
    template <typename Func>
    void each(Func&& fn)
    {
        for (size_t i = 0; i < m_dense.size(); ++i)
        {
            fn(m_entities[i], m_dense[i]);
        }
    }

    /**
     * @brief Iterates over all components (const version)
     */
    template <typename Func>
    void each(Func&& fn) const
    {
        for (size_t i = 0; i < m_dense.size(); ++i)
        {
            fn(m_entities[i], m_dense[i]);
        }
    }

    /**
     * @brief Gets number of components in storage
     */
    size_t size() const override
    {
        return m_dense.size();
    }

    /**
     * @brief Gets the type name for serialization
     */
    std::string getTypeName() const override
    {
        // Will be set via Registry's type name map
        return typeid(T).name();
    }

    /**
     * @brief Direct access to dense component array (for efficient iteration)
     */
    std::vector<T>& components()
    {
        return m_dense;
    }
    const std::vector<T>& components() const
    {
        return m_dense;
    }

    /**
     * @brief Direct access to entity array (parallel to dense components)
     */
    std::vector<Entity>& entities()
    {
        return m_entities;
    }
    const std::vector<Entity>& entities() const
    {
        return m_entities;
    }

private:
    std::vector<T>                     m_dense;     ///< Densely-packed component array
    std::vector<Entity>                m_entities;  ///< Parallel array of entity IDs
    std::unordered_map<Entity, size_t> m_sparse;    ///< Entity -> dense index mapping
};

/**
 * @brief Central registry for entity and component management
 *
 * The Registry is the single source of truth for all entities and their components.
 * It owns per-type component stores and provides O(1) expected add/remove/has/get.
 * GameEngine owns one Registry instance (single world/scene model).
 */
class Registry
{
public:
    Registry()  = default;
    ~Registry() = default;

    // Non-copyable, non-movable
    Registry(const Registry&)            = delete;
    Registry& operator=(const Registry&) = delete;

    /**
     * @brief Creates a new entity
     * @return The created entity ID
     */
    Entity createEntity()
    {
        Entity entity;
        entity.id = ++m_nextEntityId;
        m_entities.push_back(entity);
        return entity;
    }

    /**
     * @brief Destroys an entity and removes all its components
     * @param entity Entity to destroy
     */
    void destroy(Entity entity)
    {
        if (!entity.isValid())
            return;

        // Remove from all component stores
        for (auto& [typeIndex, store] : m_componentStores)
        {
            store->remove(entity);
        }

        // Remove from entity list
        auto it = std::find(m_entities.begin(), m_entities.end(), entity);
        if (it != m_entities.end())
        {
            m_entities.erase(it);
        }
    }

    /**
     * @brief Adds a component to an entity
     * @tparam T Component type
     * @param entity Entity to add component to
     * @param args Arguments forwarded to component constructor
     * @return Reference to the created component
     */
    template <typename T, typename... Args>
    T& add(Entity entity, Args&&... args)
    {
        assert(entity.isValid() && "Cannot add component to null entity");
        auto& store = getOrCreateStore<T>();
        return store.add(entity, std::forward<Args>(args)...);
    }

    /**
     * @brief Removes a component from an entity
     * @tparam T Component type
     * @param entity Entity to remove component from
     */
    template <typename T>
    void remove(Entity entity)
    {
        auto* store = getStore<T>();
        if (store)
        {
            store->remove(entity);
        }
    }

    /**
     * @brief Checks if entity has a component
     * @tparam T Component type
     * @param entity Entity to check
     * @return true if component exists
     */
    template <typename T>
    bool has(Entity entity) const
    {
        const auto* store = getStore<T>();
        return store && store->has(entity);
    }

    /**
     * @brief Gets a component (asserts if not present)
     * @tparam T Component type
     * @param entity Entity to get component from
     * @return Reference to component
     */
    template <typename T>
    T& get(Entity entity)
    {
        auto& store = getOrCreateStore<T>();
        return store.get(entity);
    }

    /**
     * @brief Gets a component (const version)
     */
    template <typename T>
    const T& get(Entity entity) const
    {
        const auto* store = getStore<T>();
        assert(store && "Component store does not exist");
        return store->get(entity);
    }

    /**
     * @brief Tries to get a component, returns nullptr if not present
     * @tparam T Component type
     * @param entity Entity to get component from
     * @return Pointer to component or nullptr
     */
    template <typename T>
    T* tryGet(Entity entity)
    {
        auto* store = getStore<T>();
        return store ? store->tryGet(entity) : nullptr;
    }

    /**
     * @brief Tries to get a component (const version)
     */
    template <typename T>
    const T* tryGet(Entity entity) const
    {
        const auto* store = getStore<T>();
        return store ? store->tryGet(entity) : nullptr;
    }

    /**
     * @brief Iterates over all entities with component T
     * @tparam T Component type
     * @param fn Callback function(EntityId, T&)
     */
    template <typename T, typename Func>
    void each(Func&& fn)
    {
        auto* store = getStore<T>();
        if (store)
        {
            store->each(std::forward<Func>(fn));
        }
    }

    /**
     * @brief Iterates over all entities with component T (const version)
     */
    template <typename T, typename Func>
    void each(Func&& fn) const
    {
        const auto* store = getStore<T>();
        if (store)
        {
            store->each(std::forward<Func>(fn));
        }
    }

    /**
     * @brief Gets all entities in the registry
     */
    const std::vector<Entity>& getEntities() const
    {
        return m_entities;
    }

    /**
     * @brief Clears all entities and components
     */
    void clear()
    {
        m_componentStores.clear();
        m_entities.clear();
        m_nextEntityId = 0;
    }

    /**
     * @brief Registers a stable type name for serialization
     * @tparam T Component type
     * @param typeName Stable string name for this type
     */
    template <typename T>
    void registerTypeName(const std::string& typeName)
    {
        std::type_index typeIdx(typeid(T));
        m_typeNames.emplace(typeIdx, typeName);
        m_nameToType.emplace(typeName, typeIdx);
    }

    /**
     * @brief Gets the stable type name for a component type
     */
    template <typename T>
    std::string getTypeName() const
    {
        auto it = m_typeNames.find(std::type_index(typeid(T)));
        return (it != m_typeNames.end()) ? it->second : typeid(T).name();
    }

    /**
     * @brief Gets type_index from a stable type name
     */
    std::type_index getTypeFromName(const std::string& typeName) const
    {
        auto it = m_nameToType.find(typeName);
        assert(it != m_nameToType.end() && "Type name not registered");
        return it->second;
    }

private:
    /**
     * @brief Gets or creates a component store for type T
     */
    template <typename T>
    ComponentStore<T>& getOrCreateStore()
    {
        std::type_index typeIdx(typeid(T));
        auto            it = m_componentStores.find(typeIdx);

        if (it == m_componentStores.end())
        {
            auto  store                = std::make_unique<ComponentStore<T>>();
            auto* storePtr             = store.get();
            m_componentStores[typeIdx] = std::move(store);
            return *storePtr;
        }

        return *static_cast<ComponentStore<T>*>(it->second.get());
    }

    /**
     * @brief Gets an existing component store for type T
     */
    template <typename T>
    ComponentStore<T>* getStore()
    {
        auto it = m_componentStores.find(std::type_index(typeid(T)));
        return (it != m_componentStores.end()) ? static_cast<ComponentStore<T>*>(it->second.get()) : nullptr;
    }

    /**
     * @brief Gets an existing component store (const version)
     */
    template <typename T>
    const ComponentStore<T>* getStore() const
    {
        auto it = m_componentStores.find(std::type_index(typeid(T)));
        return (it != m_componentStores.end()) ? static_cast<const ComponentStore<T>*>(it->second.get()) : nullptr;
    }

    uint32_t m_nextEntityId = 0;  ///< Counter for generating unique entity IDs

    std::vector<Entity> m_entities;  ///< All active entities

    /// Per-type component stores, keyed by type_index
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> m_componentStores;

    /// Stable type name mapping for serialization
    std::unordered_map<std::type_index, std::string> m_typeNames;
    std::unordered_map<std::string, std::type_index> m_nameToType;
};

#endif  // REGISTRY_H
