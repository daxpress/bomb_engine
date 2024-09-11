#pragma once

#include <entt/entt.hpp>

namespace BE_NAMESPACE
{
class Entity;

class Scene
{
public:
    Scene();
    ~Scene();
    void start();
    void update(float delta_time);
    auto spawn_entity() -> Entity;
    void destroy_entity(Entity entity);
    // destroy_entity instead of Entity destructor to avoid having to keep it alive in a
    // collection

    friend class Entity;

private:
    entt::registry m_registry;
};
}  // namespace BE_NAMESPACE