#pragma once

#include <memory>
#include <utility>

#include "Common.hpp"

struct GLFWwindow;

namespace Felina 
{
	class Window;
	class Scene;
	class Renderer;
	class Object;
	class UI;
	class Input;

	class Application
	{
		public:			
			Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight);
			~Application();

			void Init();
			void Run();
			void CleanUp();

			void LoadScene(const std::filesystem::path& filepath = DEFAULT_SCENE);

			const std::string& GetName() const { return m_name; }
			inline Window& GetWindow() { return *m_window; }
			inline Input& GetInput() { return *m_input; }
			inline Scene& GetScene() { return *m_scene; }
			
			// NOTE: it "consumes" the value when called (see Renderer.cpp)
			inline bool IsFramebufferResized() { return std::exchange(m_isFramebufferResized, false); }
			inline void SignalFramebufferResized() { m_isFramebufferResized = true; }

		private:
			static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
			static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

			void InitGlfw();
			void InitImGui();
			void Update();

			bool m_isFramebufferResized{ false };

			std::unique_ptr<Window> m_window = nullptr;
			std::unique_ptr<Input> m_input = nullptr;
			std::unique_ptr<UI> m_UI = nullptr;
			std::unique_ptr<Scene> m_scene = nullptr;
			std::unique_ptr<Renderer> m_renderer = nullptr;			
			
			const std::string m_name;
			const uint32_t m_startupWindowWidth;
			const uint32_t m_startupWindowHeight;
	};
}