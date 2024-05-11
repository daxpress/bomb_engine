#include "window.h"

#include <GLFW/glfw3.h>

namespace bomb_engine
{
	GLFWwindow* init_window(const uint32_t width, const uint32_t height, const std::string& title)
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
		:m_width(width), m_height(height), m_title(title)
	{
		glfwInit();
		m_raw_window = init_window(width, height, title);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_raw_window);
	}

	inline bool Window::is_open()
	{
		return !glfwWindowShouldClose(m_raw_window);
	}
	inline void Window::poll_events()
	{
		glfwPollEvents();
	}
}
