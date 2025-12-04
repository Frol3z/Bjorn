#include "Application.hpp"

#include "ResourceManager.hpp"
#include "Window.hpp"
#include "UI.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Felina 
{
	Application::Application(const std::string& name, uint32_t windowWidth, uint32_t windowHeight)
		: m_name(name)
	{
		InitGlfw();

		m_window = std::make_unique<Window>(windowWidth, windowHeight, m_name, *this);
		m_UI = std::make_unique<UI>();
		m_scene = std::make_unique<Scene>(windowWidth, windowHeight);
		m_renderer = std::make_unique<Renderer>(*this, *m_window, *m_scene);

		InitImGui();
		InitScene();
	}

	Application::~Application() = default;

	void Application::Run()
	{
		MainLoop();
		CleanUp();
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
		// NOTE: COORDINATE SYSTEM
		// X -> right
		// Y -> forward
		// Z -> up

		// Set camera position
		m_scene->GetCamera().SetPosition(glm::vec3(0.0f, -6.0f, 0.0f));
		
		// Load mesh
		auto& rm = ResourceManager::GetInstance(*m_renderer);
		rm.LoadMesh();

		// Create an object
		auto obj = std::make_unique<Object>("Cube", rm.GetMesh("CubeMesh"));
		Transform& t = obj->GetTransform();
		t.Rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		t.Translate(glm::vec3(-1.0f, 0.0f, 0.0f));
		m_scene->AddObject(std::move(obj));

		auto obj2 = std::make_unique<Object>("Cube (1)", rm.GetMesh("CubeMesh"));
		Transform& t2 = obj2->GetTransform();
		t2.Rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		t2.Translate(glm::vec3(1.0f, 0.0f, 0.0f));
		m_scene->AddObject(std::move(obj2));
	}

	void Application::MainLoop() 
	{
		while (!m_window->ShouldClose()) {
			glfwPollEvents();
			m_UI->Update(*m_scene);
			m_renderer->DrawFrame();
		}

		// Wait for pending GPU operations to finish
		m_renderer->WaitIdle();
	}

	void Application::CleanUp()
	{
		// Unload all resources
		auto& rm = ResourceManager::GetInstance(*m_renderer);
		rm.UnloadAll();

		// ImGui
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		// GLFW
		m_window.reset();
		glfwTerminate();
	}
}