
#include "app.h"

#include "entity.h"
#include "native_script.h"
#include "scriptable.h"

namespace BE_NAMESPACE
{
App::App(bool debug) : m_debug(debug)
{
    m_time_manager = TimeManager();
    m_window = std::make_shared<Window>(1920, 1080, "Bomb Engine");
    m_renderer = std::make_shared<Renderer>(*m_window, m_debug);

    // now ideally we would have the scene loaded from some config file, for now we create a sample
    // scene.
    m_current_scene = std::make_shared<Scene>();
}
void App::start()
{
    m_time_manager.start();

    // sample scene setup
    auto entity = m_current_scene->spawn_entity();
    entity.add_component<bomb_engine::Scriptable>(bomb_engine::NativeScript());

    m_current_scene->start();
}
void App::loop()
{
    while (m_window->is_open())
    {
        // main application loop!
        // first update delta time
        auto delta_time = m_time_manager.tick();
        // update the current scene
        m_window->poll_events();
        m_current_scene->update(delta_time);
        m_renderer->draw_frame();
        // task_graph and render_graph in the future...
        // distant future :P
    }
}
void App::exit()
{
    // clean whatever is needed to be cleaned
}
void App::restart()
{
    // surprisingly useful
    exit();
    start();
}
}  // namespace BE_NAMESPACE