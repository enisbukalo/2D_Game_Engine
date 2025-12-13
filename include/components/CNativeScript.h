#ifndef CNATIVESCRIPT_H
#define CNATIVESCRIPT_H

#include <Entity.h>
#include <memory>
#include <type_traits>
#include <utility>

class World;

namespace Components
{

class INativeScript
{
public:
    virtual ~INativeScript() = default;

    virtual void onCreate(Entity self, World& world)
    {
        (void)self;
        (void)world;
    }

    virtual void onUpdate(float deltaTime, Entity self, World& world) = 0;
};

struct CNativeScript
{
    std::unique_ptr<INativeScript> instance;
    bool                           created = false;

    template <typename T, typename... Args>
    void bind(Args&&... args)
    {
        static_assert(std::is_base_of<INativeScript, T>::value,
                      "CNativeScript::bind requires T to derive from INativeScript");
        instance = std::make_unique<T>(std::forward<Args>(args)...);
        created  = false;
    }

    bool isBound() const
    {
        return static_cast<bool>(instance);
    }
};

}  // namespace Components

#endif  // CNATIVESCRIPT_H
