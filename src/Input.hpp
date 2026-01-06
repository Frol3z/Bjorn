#pragma once

#include <utility>

namespace Felina
{
	class Window;

	class Input
	{
		public:
			static constexpr float MOUSE_SENSITIVITY = 100.0f;
			static constexpr float SCROLL_SENSITIVITY = 0.5f;
			static constexpr float PAN_SENSITIVITY = 0.05f;

			double mouseDeltaX{ 0.0 };
			double mouseDeltaY{ 0.0 };

			Input() = default;
			void Update(const Window& window);

			inline double GetMouseScroll() { return std::exchange(m_mouseScroll, 0.0); }
			inline void SetMouseScroll(double amount) { m_mouseScroll = amount * SCROLL_SENSITIVITY; }

		private:
			// Normalized [0,1] mouse input
			double m_mouseX{ 0.0 };
			double m_mouseY{ 0.0 };
			double m_mouseScroll{ 0.0 };
	};
}