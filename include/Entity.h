#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>
#include <functional>

/**
 * @brief Lightweight entity identifier - just an integer ID
 *
 * Entity is now a simple ID-only handle (uint32_t wrapper) without versioning or generation.
 * The Registry manages all component storage and lifecycle.
 *
 * This replaces the old Entity class entirely. Components are no longer owned by entities;
 * instead, the Registry owns all component storage using dense arrays with sparse entity->index mapping.
 */
struct Entity
{
    uint32_t id = 0;

    /**
     * @brief Creates a null/invalid entity
     */
    static constexpr Entity null()
    {
        return Entity{0};
    }

    /**
     * @brief Checks if this is a valid entity
     */
    constexpr bool isValid() const
    {
        return id != 0;
    }

    /**
     * @brief Allows entity to be used in boolean context
     */
    constexpr explicit operator bool() const
    {
        return isValid();
    }

    // Equality operators
    constexpr bool operator==(const Entity& other) const
    {
        return id == other.id;
    }
    constexpr bool operator!=(const Entity& other) const
    {
        return id != other.id;
    }
    constexpr bool operator<(const Entity& other) const
    {
        return id < other.id;
    }
};

// Hash function for Entity to use in unordered_map/set
namespace std
{
template <>
struct hash<Entity>
{
    size_t operator()(const Entity& entity) const noexcept
    {
        return std::hash<uint32_t>{}(entity.id);
    }
};
}  // namespace std

#endif  // ENTITY_H
