#pragma once

#include <string>

// Fwd declaration
struct GLFWwindow;

namespace Felina 
{
	// Fwd declaration
	class Application;

	class Window 
	{
		public:
			Window(uint32_t width, uint32_t height, const std::string& title, Application& app);
			~Window();

			void SetWidth(uint32_t width) { m_width = width; }
			void SetHeight(uint32_t height) { m_height = height; }
			uint32_t GetWidth() const { return m_width; }
			uint32_t GetHeight() const { return m_height; }

			GLFWwindow* GetHandle() const { return m_window; }
			int ShouldClose() const;

		private:
			static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
			
			GLFWwindow* m_window = nullptr;
			Application& m_app; // Required to notify the app when the window is resized
			uint32_t m_width;
			uint32_t m_height;
	};
}