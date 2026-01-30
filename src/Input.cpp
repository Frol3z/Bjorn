#include "Input.hpp"

#include "Window.hpp"
#include <glfw/glfw3.h>

namespace Felina
{
	// Process mouse input and updates member variables m_mouseDeltaX and m_mouseDeltaY
	void Input::Update(const Window& window)
	{
		// xpos, ypos -> mouse position at frame N
		// m_mouseX, m_mouseY -> mouse position at frame N-1

		// Poll for mouse position
		double xpos{};
		double ypos{};
		glfwGetCursorPos(window.GetHandle(), &xpos, &ypos);
		xpos /= window.GetWidth();
		ypos /= window.GetHeight();

		// Compute delta
		mouseDeltaX = (m_mouseX - xpos) * MOUSE_SENSITIVITY;
		mouseDeltaY = (m_mouseY - ypos) * MOUSE_SENSITIVITY;

		// Update stored mouse position
		m_mouseX = xpos;
		m_mouseY = ypos;
	}
}