#include "SScript.h"

#include <CNativeScript.h>
#include <World.h>

#include <vector>

void Systems::SScript::update(float deltaTime, World& world)
{
    // Snapshot entities first so scripts can safely spawn entities / add scripts
    // without invalidating the underlying component store iteration.
    std::vector<Entity> scriptedEntities;
    scriptedEntities.reserve(64);

    world.components().view<Components::CNativeScript>([&](Entity entity, Components::CNativeScript& /*script*/)
                                                       { scriptedEntities.push_back(entity); });

    for (Entity entity : scriptedEntities)
    {
        if (!world.isAlive(entity))
        {
            continue;
        }

        auto* script = world.components().tryGet<Components::CNativeScript>(entity);
        if (!script || !script->instance)
        {
            continue;
        }

        if (!script->created)
        {
            script->instance->onCreate(entity, world);
            script->created = true;
        }

        script->instance->onUpdate(deltaTime, entity, world);
    }
}
