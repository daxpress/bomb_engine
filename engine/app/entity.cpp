#include "entity.h"

#include "scene.h"

namespace BE_NAMESPACE
{
Entity::Entity(Scene& scene) : m_scene_ref(scene) { m_entity = scene.m_registry.create(); }
}  // namespace BE_NAMESPACE