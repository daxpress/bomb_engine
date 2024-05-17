#include "scene.h"
#include "entity.h"
#include "scriptable.h"

namespace BE_NAMESPACE
{
	Scene::Scene()
	{
	}
	Scene::~Scene()
	{
		m_registry.clear();
	}

	void Scene::start()
	{
		auto scripts = m_registry.view<Scriptable>();
		for (auto&& [entity, script] : scripts.each())
		{
			script->start();
		}
	}

	void Scene::update(float tick)
	{
		auto scripts = m_registry.view<Scriptable>();
		for (auto&& [entity, script] : scripts.each())
		{
			script->update(tick);
		}
	}

	Entity Scene::spawn_entity()
	{
		return Entity(*this);
	}
}
