#include "Application.hpp"

#include "ResourceManager.hpp"
#include "Window.hpp"
#include "UI.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "GltfLoader.hpp"
#include "Input.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Felina
{
	static const std::filesystem::path DEFAULT_SCENE{ "./assets/pbr.glb" };
	static const std::filesystem::path DEFAULT_SKYBOX{ "./assets/skybox/" };

	void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		assert(app != nullptr && "[Application] glfwGetWindowUserPointer hasn't been set.");

		// Update window
		Window& win = app->GetWindow();
		win.SetWidth(static_cast<uint32_t>(width));
		win.SetHeight(static_cast<uint32_t>(height));

		// Update camera viewport
		Camera& camera = app->GetScene().GetCamera();
		camera.UpdateProjectionMatrix(static_cast<float>(width), static_cast<float>(height));

		// Signal the renderer
		app->SignalFramebufferResized();
	}

	// For a classic vertical mouse-wheel xOffset should be ignored 
	void Application::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
	{
		Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		assert(app != nullptr && "[Application] glfwGetWindowUserPointer hasn't been set.");

		app->GetInput().SetMouseScroll(0.35f * yOffset); // WIP
	}

	Application::Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight)
		: m_name(name), m_isFramebufferResized(false), m_startupWindowWidth(windowWidth), m_startupWindowHeight(windowHeight)
	{
	}

	Application::~Application() = default;

	void Application::Init()
	{
		InitGlfw();
		m_window = std::make_unique<Window>(m_startupWindowWidth, m_startupWindowHeight, m_name);
		glfwSetWindowUserPointer(m_window->GetHandle(), this);
		glfwSetFramebufferSizeCallback(m_window->GetHandle(), Application::FramebufferResizeCallback);
		glfwSetScrollCallback(m_window->GetHandle(), Application::ScrollCallback);

		m_input = std::make_unique<Input>();
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
			m_input->Update(*m_window);
			UpdateCamera();
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
		LOG("[Application] Loading skybox...");
		m_renderer->LoadSkybox(DEFAULT_SKYBOX);
		LOG("[Application] Skybox loaded successfully!");

		LOG("[Application] Loading default scene...");
		LoadSceneFromGlTF(DEFAULT_SCENE, *m_scene, *m_renderer);
		// TODO: include camera in the glTF
		m_scene->GetCamera().SetPosition(glm::vec3(0.0f, -6.0f, 3.0f));
		LOG("[Application] Default scene loaded successfully!");
	}

	void Application::UpdateCamera()
	{
		// TODO: 
		// - fix the double sensitivity issue
		// - expose sensitivity in the UI
		auto& camera = m_scene->GetCamera();
		GLFWwindow* win = m_window->GetHandle();

		// Mouse buttons -> Panning and orbiting
		if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			camera.Rotate(m_input->mouseDeltaX, m_input->mouseDeltaY);
		else if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			camera.Pan(Input::PAN_SENSITIVITY * m_input->mouseDeltaX, Input::PAN_SENSITIVITY * m_input->mouseDeltaY);

		// Mouse scroll-wheel -> Dolly
		double mouseScroll = m_input->GetMouseScroll();
		if (mouseScroll != 0.0)
			camera.Dolly(mouseScroll);
	}
}