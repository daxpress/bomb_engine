#pragma once

#include "renderer.h"
#include "scene.h"
#include "time_manager.h"

namespace BE_NAMESPACE
{
class App
{
public:
    explicit App(bool debug = false);
    ~App() = default;

    void start();
    void loop();
    void exit();
    void restart();

    inline auto scene() -> std::shared_ptr<Scene> { return m_current_scene; }
    // the idea is to always guarantee that this will not be null (except for default before
    // creation)

private:
    // specific for each application
    TimeManager m_time_manager;

    // can be shared and/or changed
    std::shared_ptr<Window> m_window = nullptr;
    std::shared_ptr<Renderer> m_renderer = nullptr;
    std::shared_ptr<Scene> m_current_scene = nullptr;

    // config
    bool m_debug = false;
};
}  // namespace BE_NAMESPACE