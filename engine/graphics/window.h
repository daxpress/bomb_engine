#pragma once

class GLFWwindow;

namespace BE_NAMESPACE
{
class Window
{
public:
    /// <summary>
    /// Creates a window with primary monitor resolution size and engine title
    /// </summary>
    Window();
    Window(const uint32_t width, const uint32_t height, const std::string& title);
    ~Window();

    auto is_open() -> bool;
    void poll_events();

    [[nodiscard]] inline auto get_title() const -> const std::string& { return m_title; }
    [[nodiscard]] inline auto get_raw_window() const -> GLFWwindow* { return m_raw_window; }

private:
    GLFWwindow* m_raw_window = nullptr;
    uint32_t m_width;
    uint32_t m_height;
    std::string m_title;
};
}  // namespace BE_NAMESPACE