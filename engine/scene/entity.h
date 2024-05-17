#pragma once
#include "scene.h"

#include <entt/entt.hpp>

namespace BE_NAMESPACE
{
	// small wrapper to allow easier entity allocation and management for users
	class Entity
	{
	public:
		Entity(Scene& scene);
		~Entity();

		template<typename Component, typename... Args>
		Component add_component(Args&&... args) 
		{
			return m_scene_ref.m_registry.emplace<Component>(m_entity, std::forward<Args>(args)...);
		};

		template<typename Component>
		void remove_component(Component component)
		{
			m_scene_ref.m_registry.remove<Component>(m_entity);
		}

		template<typename Component>
		Component get_component()
		{
			return m_scene_ref.m_registry.try_get<Component>();
		}

		friend class Scene;

	private:
		entt::entity m_entity;
		Scene& m_scene_ref;
	};
}