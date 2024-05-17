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
		Entity spawn_entity();

		friend class Entity;

	private:
		entt::registry m_registry;
	};
}