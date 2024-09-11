#pragma once
#include <entt/entt.hpp>

#include "scene.h"

namespace BE_NAMESPACE
{
// small wrapper to allow easier entity allocation and management for users
class Entity
{
public:
    explicit Entity(Scene& scene);
    ~Entity() = default;

    template <typename Component, typename... Args>
    auto add_component(Args&&... args) -> Component
    {
        return m_scene_ref.m_registry.emplace<Component>(m_entity, std::forward<Args>(args)...);
    };

    template <typename Component>
    void remove_component(Component component)
    {
        m_scene_ref.m_registry.remove<Component>(m_entity);
    }

    template <typename... Components>
    auto get_component()
    {
        return m_scene_ref.m_registry.try_get<Components...>();
    }

    // not explicit to facilitate some operations
    operator entt::entity() const { return m_entity; }

    friend class Scene;

private:
    entt::entity m_entity;
    Scene& m_scene_ref;
};
}  // namespace BE_NAMESPACE