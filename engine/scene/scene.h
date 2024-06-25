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
    void update(float tick);
    auto spawn_entity() -> Entity;

    friend class Entity;

private:
    entt::registry m_registry;
};
}  // namespace BE_NAMESPACE