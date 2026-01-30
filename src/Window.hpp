#pragma once

#include <string>

// Fwd declaration
struct GLFWwindow;

namespace Felina 
{
	class Window 
	{
		public:
			Window(uint32_t width, uint32_t height, const std::string& title);
			~Window();

			inline void SetWidth(uint32_t width) { m_width = width; }
			inline void SetHeight(uint32_t height) { m_height = height; }
			inline uint32_t GetWidth() const { return m_width; }
			inline uint32_t GetHeight() const { return m_height; }

			inline GLFWwindow* GetHandle() const { return m_window; }
			bool ShouldClose() const;

		private:
			GLFWwindow* m_window = nullptr;
			uint32_t m_width;
			uint32_t m_height;
	};
}