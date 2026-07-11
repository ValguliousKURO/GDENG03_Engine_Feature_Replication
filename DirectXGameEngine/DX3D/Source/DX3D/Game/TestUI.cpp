#include <DX3D/Game/TestUI.h>

#include <DX3D/Game/World.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Component/CubeComponent.h>

#include <DX3D/Component/CameraComponent.h>
#include <DX3D/Graphics/GraphicsDevice.h>

// ImGui's DragFloat3 range uses FLT_MAX.
#include <cfloat>
#include <imgui.h>

dx3d::TestUI::TestUI(const BaseDesc& desc) :Base(desc)
{


}

void dx3d::TestUI::draw(GameObject& object)
{

	if (ImGui::Begin("Test UI"))
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

dx3d::TestUI::~TestUI()
{


}

void dx3d::TestUI::render(World& world, GraphicsDevice& graphicsDevice, SwapChain& swapChain)
{
}

void dx3d::TestUI::drawGameObjectPanel(World& world)
{
}

void dx3d::TestUI::drawTransformInspector(GameObject& object) // draw the inspector tab
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

void dx3d::TestUI::drawComponentInspector(GameObject& object)
{
	ImGui::Text("Components inspector - expand to list and edit components. Not out");

}
