#include "Application.hpp"

#include "ResourceManager.hpp"
#include "Window.hpp"
#include "UI.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "GltfLoader.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Felina
{
	static const std::filesystem::path DEFAULT_SCENE{"./assets/multi_textured_scene.gltf"};

	Application::Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight)
		: m_name(name), m_startupWindowWidth(windowWidth), m_startupWindowHeight(windowHeight)
	{
	}

	Application::~Application() = default;

	void Application::Init()
	{
		InitGlfw();

		m_window = std::make_unique<Window>(m_startupWindowWidth, m_startupWindowHeight, m_name, *this);
		m_UI = std::make_unique<UI>();
		m_scene = std::make_unique<Scene>(static_cast<float>(m_startupWindowWidth), static_cast<float>(m_startupWindowHeight));
		m_renderer = std::make_unique<Renderer>(*this, *m_window, *m_scene);

		InitImGui();
		InitScene();

		m_renderer->UpdateDescriptorSets();
	}

	void Application::Run()
	{
		while (!m_window->ShouldClose()) {
			glfwPollEvents();
			ProcessInput();

			if(glfwGetMouseButton(m_window->GetHandle(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
				m_scene->RotateCamera(m_mouseDeltaX, m_mouseDeltaY);
			
			m_UI->Update(*m_scene);
			m_renderer->DrawFrame();
		}

		// Wait for pending GPU operations to finish
		m_renderer->WaitIdle();		
	}

	void Application::CleanUp()
	{
		// Unload all resources
		auto& rm = ResourceManager::GetInstance();
		rm.UnloadAll();

		// ImGui
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		// GLFW
		m_window.reset();
		glfwTerminate();
	}

	void Application::InitGlfw() 
	{
		glfwInit();
	}

	void Application::InitImGui()
	{
		ImGui::CreateContext();

		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// Init backends
		ImGui_ImplGlfw_InitForVulkan(m_window->GetHandle(), true); // Second parameter is for chaining input callbacks to ImGui automatically
		ImGui_ImplVulkan_InitInfo vkInitInfo = m_renderer->GetImGuiInitInfo();
		ImGui_ImplVulkan_Init(&vkInitInfo);
	}

	void Application::InitScene()
	{	
		LOG("[Application] Loading default scene..");

		// Load default scene
		LoadSceneFromGlTF(DEFAULT_SCENE, *m_scene, *m_renderer);
		
		// TODO: include camera in the glTF
		m_scene->GetCamera().SetPosition(glm::vec3(0.0f, -6.0f, 3.0f));
		
		LOG("[Application] Default scene loaded successfully!");
	}

	// Process mouse input and updates member variables
	// m_mouseDeltaX and m_mouseDeltaY
	void Application::ProcessInput()
	{
		// xpos, ypos -> mouse position at frame N
		// m_mouseX, m_mouseY -> mouse position at frame N-1

		// Poll for mouse position
		double xpos{};
		double ypos{};
		glfwGetCursorPos(m_window->GetHandle(), &xpos, &ypos);
		xpos /= m_window->GetWidth();
		ypos /= m_window->GetHeight();

		// Compute delta
		m_mouseDeltaX = (m_mouseX - xpos) * m_sensitivity;
		m_mouseDeltaY = (m_mouseY - ypos) * m_sensitivity;

		// Update stored mouse position
		m_mouseX = xpos;
		m_mouseY = ypos;
	}
}