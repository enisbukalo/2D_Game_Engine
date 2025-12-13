#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>
#include <functional>

/**
 * @brief Entity handle with index + generation for stale-handle detection
 */
struct Entity
{
    uint32_t index{0};
    uint32_t generation{0};

    constexpr Entity() = default;
    constexpr Entity(uint32_t idx, uint32_t gen = 0) : index(idx), generation(gen) {}

    /**
     * @brief Creates a null/invalid entity
     */
    static constexpr Entity null()
    {
        return Entity{0, 0};
    }

    /**
     * @brief Checks if this is a non-null handle (liveness is validated by EntityManager)
     */
    constexpr bool isValid() const
    {
        return index != 0;
    }

    /**
     * @brief Allows entity to be used in boolean context
     */
    constexpr explicit operator bool() const
    {
        return isValid();
    }

    // Equality operators compare both index and generation
    constexpr bool operator==(const Entity& other) const
    {
        return index == other.index && generation == other.generation;
    }
    constexpr bool operator!=(const Entity& other) const
    {
        return !(*this == other);
    }
    constexpr bool operator<(const Entity& other) const
    {
        return index < other.index || (index == other.index && generation < other.generation);
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
        // Mix index and generation without relying on pointer size
        return (static_cast<size_t>(entity.index) * 0x9e3779b1u) ^ static_cast<size_t>(entity.generation);
    }
};
}  // namespace std

#endif  // ENTITY_H
