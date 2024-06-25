#include "window.h"

namespace BE_NAMESPACE
{
auto init_window(const uint32_t width, const uint32_t height, const std::string& title)
    -> GLFWwindow*
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    return glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

Window::Window()
{
    glfwInit();
    const auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    m_width = static_cast<uint32_t>(mode->width);
    m_height = static_cast<uint32_t>(mode->height);
    m_title = "Bomb Engine";
    m_raw_window = init_window(m_width, m_height, m_title);
}

Window::Window(const uint32_t width, const uint32_t height, const std::string& title)
    : m_width(width), m_height(height), m_title(title)
{
    glfwInit();
    m_raw_window = init_window(width, height, title);
}

Window::~Window() { glfwDestroyWindow(m_raw_window); }

auto Window::is_open() -> bool { return !glfwWindowShouldClose(m_raw_window); }
void Window::poll_events() { glfwPollEvents(); }
}  // namespace BE_NAMESPACE
