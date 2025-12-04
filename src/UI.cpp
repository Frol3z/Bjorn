#include "UI.hpp"

#include "Scene.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <iostream>

namespace Felina
{
	void UI::Update(Scene& scene)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

		//ImGui::ShowDemoWindow();

		// Custom UI
		DrawHierarchy(scene);
		DrawInspector();

		ImGui::Render();
	}

	void UI::DrawHierarchy(Scene& scene)
	{
		ImGui::Begin("Hierarchy");
		ImGuiTreeNodeFlags baseFlags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_Leaf |
			ImGuiTreeNodeFlags_NoTreePushOnOpen;
		size_t i = 0;
		for (auto obj : scene.GetObjects())
		{
			const bool isSelected = (m_hierarchySelection == obj);
			ImGuiTreeNodeFlags flags = baseFlags | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);
			ImGui::TreeNodeEx((void*)(intptr_t)i, flags, obj->GetName().c_str());
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				m_hierarchySelection = obj;

				// Initialize inspector values
				Transform& t = obj->GetTransform();
				m_displayedPosition = t.GetPosition();
				m_displayedRotation = glm::degrees(glm::eulerAngles(t.GetRotation()));
				m_displayedScale = t.GetScale();
			}
			i++;
		}
		ImGui::End();
	}

	void UI::DrawInspector()
	{
		ImGui::Begin("Inspector");
		if (m_hierarchySelection)
		{
			ImGui::SeparatorText(m_hierarchySelection->GetName().c_str());

			Transform& t = m_hierarchySelection->GetTransform();

			if (ImGui::DragFloat3("Position", &m_displayedPosition.x, 0.01f))
			{
				t.SetPosition(m_displayedPosition);
			}

			if (ImGui::DragFloat3("Rotation", &m_displayedRotation.x, 1.0f))
			{
				glm::quat targetRotation = glm::quat(glm::radians(m_displayedRotation));
				t.SetRotation(targetRotation);
			}

			if (ImGui::DragFloat3("Scale", &m_displayedScale.x, 0.01f))
			{
				t.SetScale(m_displayedScale);
			}
		}
		ImGui::End();
	}
}