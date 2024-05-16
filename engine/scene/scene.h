#pragma once

#include <entt/entt.hpp>

namespace BE_NAMESPACE
{
	class Scene
	{
	public:

		Scene() = default;
		void Update();

	private:
		entt::registry m_registry;

	};
}