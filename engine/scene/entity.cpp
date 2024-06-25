#include "entity.h"

#include "scene.h"

namespace BE_NAMESPACE
{
Entity::Entity(Scene& scene) : m_scene_ref(scene) { m_entity = scene.m_registry.create(); }
Entity::~Entity() { m_scene_ref.m_registry.destroy(m_entity); }
}  // namespace BE_NAMESPACE