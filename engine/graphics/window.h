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

		bool is_open();
		void poll_events();

		inline const std::string& get_title() const { return m_title; }
		inline GLFWwindow* get_raw_window() const { return m_raw_window; }

	private:

		GLFWwindow* m_raw_window = nullptr;
		uint32_t m_width;
		uint32_t m_height;
		std::string m_title;
	};
}