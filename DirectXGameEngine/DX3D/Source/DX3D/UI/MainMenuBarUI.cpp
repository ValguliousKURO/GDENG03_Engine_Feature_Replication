#include <DX3D/UI/MainMenuBarUI.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>

// ImGui's DragFloat3 range uses FLT_MAX.
#include <cfloat>
#include <imgui.h>

dx3d::MainMenuBarUI::MainMenuBarUI(const BaseDesc& desc) : BaseUI(desc)
{
}

dx3d::MainMenuBarUI::~MainMenuBarUI()
{
}

void dx3d::MainMenuBarUI::draw()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Add New Window", "Ctrl+N"))
			{
				EventBroadcastManager::getInstance().postEvent(EventNames::ON_WINDOW_NEW);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
