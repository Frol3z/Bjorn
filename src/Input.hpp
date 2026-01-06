#pragma once

#include <utility>

namespace Felina
{
	class Window;

	class Input
	{
		public:
			double mouseDeltaX{ 0.0 };
			double mouseDeltaY{ 0.0 };
			float sensitivity{ 100.0f };

			Input() = default;
			void Update(const Window& window);

			inline double GetMouseScroll() { return std::exchange(m_mouseScroll, 0.0); }
			inline void SetMouseScroll(double amount) { m_mouseScroll = amount; }

		private:
			// Normalized [0,1] mouse input
			double m_mouseX{ 0.0 };
			double m_mouseY{ 0.0 };
			double m_mouseScroll{ 0.0 };
	};
}