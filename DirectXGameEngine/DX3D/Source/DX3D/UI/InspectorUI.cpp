#include <DX3D/UI/InspectorUI.h>

#include <DX3D/Game/World.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Component/CubeComponent.h>

#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Game/Display.h>
#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <string>

// ImGui's DragFloat3 range uses FLT_MAX.
#include <cfloat>
#include <imgui.h>

dx3d::InspectorUI::InspectorUI(const BaseDesc& desc) :BaseUI(desc)
{


}

void dx3d::InspectorUI::draw(GameObject& object, Display& display)
{
	std::string windowTitle = "Inspector UI - Window " + std::to_string(display.getID()) + "###InspectorUI_" + std::to_string(display.getID());

	ImGui::SetNextWindowSize(ImVec2(430.0f, 360.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(430.0f, 300.0f), ImVec2(FLT_MAX, FLT_MAX));
	if (ImGui::Begin(windowTitle.c_str()))
	{
		if (ImGui::BeginTabBar("##TestTabs")) // create tab bar with id
		{
			if (ImGui::BeginTabItem("Transform"))
			{
				drawTransformInspector(object);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Components"))
			{
				drawComponentInspector(object);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Viewports"))
			{
				drawViewportPanel(display);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Scene"))
			{
				// optional future scene / object list
				ImGui::Text("Scene / gameobject list can go here.");
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar(); // end tab 
		}
	}
	ImGui::End(); // End Test UI window


}

void dx3d::InspectorUI::draw()
{
}

dx3d::InspectorUI::~InspectorUI()
{


}

void dx3d::InspectorUI::drawViewportPanel(Display& display)
{
	ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "Current Viewport");
	ImGui::Separator();

	ImGui::PushID(display.getID());
		
		std::string label = "Viewport (Window ID: " + std::to_string(display.getID()) + ")";
		if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			// View Mode: Perspective vs. Top Down
			auto* camera = display.getCamera();
			if (camera)
			{
				auto mode = camera->getProjectionMode();
				const char* modeNames[] = { "Perspective View", "Top Down View" };
				int currentModeIndex = (mode == ProjectionMode::Perspective) ? 0 : 1;
				
				ImGui::Text("Camera Settings:");
				if (ImGui::Combo("View Mode", &currentModeIndex, modeNames, IM_ARRAYSIZE(modeNames)))
				{
					if (currentModeIndex == 0)
					{
						EventBroadcastManager::getInstance().postEvent(EventNames::PERSPECTIVE_MODE_TOGGLE + "_" + std::to_string(display.getID()));
					}
					else
					{
						EventBroadcastManager::getInstance().postEvent(EventNames::ORTHOGRAPHIC_MODE_TOGGLE + "_" + std::to_string(display.getID()));
					}
				}

				if (mode == ProjectionMode::Orthographic)
				{
					float zoom = camera->getOrthoZoom();
					if (ImGui::DragFloat("Orthographic Zoom", &zoom, 0.001f, 0.005f, 0.1f, "%.4f"))
					{
						camera->setOrthoZoom(zoom);
					}
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No camera attached to this viewport.");
			}

			ImGui::Spacing();
			// Render Mode: Lit vs. Wireframe
			ImGui::Text("Render Settings:");
			int renderModeIndex = (display.getRenderMode() == Display::RenderMode::Lit) ? 0 : 1;
			const char* renderModes[] = { "Lit (Default)", "Wireframe" };
			if (ImGui::Combo("Render Mode", &renderModeIndex, renderModes, IM_ARRAYSIZE(renderModes)))
			{
				if (renderModeIndex == 0)
				{
					EventBroadcastManager::getInstance().postEvent(EventNames::LIT_MODE_TOGGLE + "_" + std::to_string(display.getID()));
				}
				else
				{
					EventBroadcastManager::getInstance().postEvent(EventNames::WIREFRAME_MODE_TOGGLE + "_" + std::to_string(display.getID()));
				}
			}
			
			// Show info warning that Wireframe is active
			if (renderModeIndex == 1)
			{
				ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Note: Wireframe mode is active.");
			}
		}

	ImGui::PopID();
	ImGui::Separator();
}

void dx3d::InspectorUI::render(World& world, GraphicsDevice& graphicsDevice, SwapChain& swapChain)
{
}

void dx3d::InspectorUI::drawGameObjectPanel(World& world)
{
}

void dx3d::InspectorUI::drawTransformInspector(GameObject& object) // draw the inspector tab
{
	auto& transform = object.getTransform();

	auto pos = transform.getPosition();
	auto rot = transform.getRotation();
	auto scale = transform.getScale();

	float p[3] = { pos.x, pos.y, pos.z };
	float r[3] = { rot.x, rot.y, rot.z };
	float s[3] = { scale.x, scale.y, scale.z };

	// Position
	if (ImGui::DragFloat3("Position", p, 0.05f, -FLT_MAX, FLT_MAX, "%.3f"))
	{
		transform.setPosition({ p[0], p[1], p[2] });
	}

	// Rotation 
	if (ImGui::DragFloat3("Rotation (rad)", r, 0.01f, -FLT_MAX, FLT_MAX, "%.3f"))
	{
		transform.setRotation({ r[0], r[1], r[2] });
	}

	// Scale
	if (ImGui::DragFloat3("Scale", s, 0.01f, 0.0f, FLT_MAX, "%.3f"))
	{
		transform.setScale({ s[0], s[1], s[2] });
	}
}

void dx3d::InspectorUI::drawComponentInspector(GameObject& object)
{
	ImGui::Text("Components inspector - expand to list and edit components. Not out");

}
