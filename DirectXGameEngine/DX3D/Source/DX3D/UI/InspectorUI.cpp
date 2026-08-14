#include <DX3D/UI/InspectorUI.h>

#include <DX3D/Game/World.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Component/TransformComponent.h>
#include <DX3D/Component/CubeComponent.h>
#include <DX3D/Component/MeshComponent.h>
#include <DX3D/Component/PhysicsComponent.h>
#include <DX3D/Component/PointLightComponent.h>

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
	EventBroadcastManager::getInstance().addObserver
	(
		EventNames::ON_GAMEOBJECT_SELECTED,
		[this](dx3d::Parameters& params)
		{
			m_selectedGameObject = params.GetGameObjectPtr("Selected", NULL);
		}
	);
	EventBroadcastManager::getInstance().addObserver
	(
		EventNames::ON_EDITOR_PLAY_MODE_CHANGED,
		[this](dx3d::Parameters& params)
		{
			m_isPlayMode = params.GetBoolExtra("IsPlayMode", false);
		}
	);
}

void dx3d::InspectorUI::draw()
{
	// std::string windowTitle = "Inspector UI - Window " + std::to_string(display.getID()) + "###InspectorUI_" + std::to_string(display.getID());

	ImGui::SetNextWindowSize(ImVec2(430.0f, 360.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(430.0f, 300.0f), ImVec2(FLT_MAX, FLT_MAX));
	//if (ImGui::Begin(windowTitle.c_str()))
	if (ImGui::Begin("Inspector"))
	{
		// Make sure we actually have a list set
		if (!m_selectedGameObject || m_selectedGameObject == NULL || m_selectedGameObject->isDeleted())
		{
			ImGui::Text("No game object selected.");
			ImGui::End();
			return;
		}

		ImGui::Text("Selected: %s", m_selectedGameObject->getName().c_str());
		ImGui::SameLine();
		ImGui::BeginDisabled(m_isPlayMode);
		if (ImGui::SmallButton("Delete Selected"))
		{
			Parameters params;
			params.PutExtra("Target", m_selectedGameObject);
			EventBroadcastManager::getInstance().postEvent(EventNames::ON_DELETE_GAMEOBJECT, params);
			m_selectedGameObject = nullptr;
			ImGui::EndDisabled();
			ImGui::End();
			return;
		}
		ImGui::EndDisabled();
		ImGui::Separator();

		if (ImGui::BeginTabBar("##InspectorTabs")) // create tab bar with id
		{
			if (ImGui::BeginTabItem("Transform"))
			{
				drawTransformInspector(*m_selectedGameObject);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Texture"))
			{
				m_textComInspector.draw(m_selectedGameObject->createOrGetComponent<MeshComponent>()->getMaterial());
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Components"))
			{
				drawComponentInspector(*m_selectedGameObject);
				ImGui::EndTabItem();
			}

			/*if (ImGui::BeginTabItem("Viewports"))
			{
				drawViewportPanel(display);
				ImGui::EndTabItem();
			}*/

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

dx3d::InspectorUI::~InspectorUI()
{
	EventBroadcastManager::getInstance().RemoveObserver(EventNames::ON_GAMEOBJECT_SELECTED);
	EventBroadcastManager::getInstance().RemoveObserver(EventNames::ON_EDITOR_PLAY_MODE_CHANGED);
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
	if (m_isPlayMode)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.25f, 1.0f), "Play Mode: object manipulation is locked.");
	}

	bool enabled = object.isEnabled();
	ImGui::BeginDisabled(m_isPlayMode);
	if (ImGui::Checkbox("Enabled", &enabled))
	{
		Parameters params;
		params.PutExtra("Target", &object);
		params.PutExtra("Enabled", enabled);
		EventBroadcastManager::getInstance().postEvent(EventNames::ON_SET_GAMEOBJECT_ENABLED, params);
	}
	ImGui::EndDisabled();
	ImGui::Separator();

	auto& transform = object.getTransform();

	auto pos = transform.getPosition();
	auto rot = transform.getRotation();
	auto scale = transform.getScale();

	float p[3] = { pos.x, pos.y, pos.z };
	float r[3] = { rot.x, rot.y, rot.z };
	float s[3] = { scale.x, scale.y, scale.z };

	ImGui::BeginDisabled(m_isPlayMode || !object.isEnabled());

	// Position
	if (ImGui::DragFloat3("Position", p, 0.05f, -FLT_MAX, FLT_MAX, "%.3f"))
	{
		transform.setPosition({ p[0], p[1], p[2] });
	}
	if (ImGui::IsItemActivated())
	{
		m_trackingPositionEdit = true;
		m_editStartPosition = pos;
	}
	if (ImGui::IsItemDeactivatedAfterEdit() && m_trackingPositionEdit)
	{
		Parameters params;
		params.PutExtra("Target", &object);
		params.PutExtra("Property", "Position");
		params.PutExtra("OldValue", m_editStartPosition);
		params.PutExtra("NewValue", transform.getPosition());
		EventBroadcastManager::getInstance().postEvent(EventNames::ON_TRANSFORM_CHANGED, params);
		m_trackingPositionEdit = false;
	}

	// Rotation 
	if (ImGui::DragFloat3("Rotation (rad)", r, 0.01f, -FLT_MAX, FLT_MAX, "%.3f"))
	{
		transform.setRotation({ r[0], r[1], r[2] });
	}
	if (ImGui::IsItemActivated())
	{
		m_trackingRotationEdit = true;
		m_editStartRotation = rot;
	}
	if (ImGui::IsItemDeactivatedAfterEdit() && m_trackingRotationEdit)
	{
		Parameters params;
		params.PutExtra("Target", &object);
		params.PutExtra("Property", "Rotation");
		params.PutExtra("OldValue", m_editStartRotation);
		params.PutExtra("NewValue", transform.getRotation());
		EventBroadcastManager::getInstance().postEvent(EventNames::ON_TRANSFORM_CHANGED, params);
		m_trackingRotationEdit = false;
	}

	// Scale
	if (ImGui::DragFloat3("Scale", s, 0.01f, 0.0f, FLT_MAX, "%.3f"))
	{
		transform.setScale({ s[0], s[1], s[2] });
	}
	if (ImGui::IsItemActivated())
	{
		m_trackingScaleEdit = true;
		m_editStartScale = scale;
	}
	if (ImGui::IsItemDeactivatedAfterEdit() && m_trackingScaleEdit)
	{
		Parameters params;
		params.PutExtra("Target", &object);
		params.PutExtra("Property", "Scale");
		params.PutExtra("OldValue", m_editStartScale);
		params.PutExtra("NewValue", transform.getScale());
		EventBroadcastManager::getInstance().postEvent(EventNames::ON_TRANSFORM_CHANGED, params);
		m_trackingScaleEdit = false;
	}

	ImGui::EndDisabled();
}

void dx3d::InspectorUI::drawComponentInspector(GameObject& object)
{
	auto* physicsComponent = object.getComponent<PhysicsComponent>();
	auto* pointLightComponent = object.getComponent<PointLightComponent>();
	const bool canAttachRigidBody = !object.getComponent<CameraComponent>() && !pointLightComponent;

	ImGui::BeginDisabled(m_isPlayMode || !object.isEnabled() || !canAttachRigidBody);
	if (!physicsComponent)
	{
		if (ImGui::Button("Add Rigid Body"))
		{
			Parameters params;
			params.PutExtra("Target", &object);
			EventBroadcastManager::getInstance().postEvent(EventNames::ON_ADD_RIGID_BODY, params);
		}
	}
	else
	{
		if (ImGui::Button("Remove Rigid Body"))
		{
			Parameters params;
			params.PutExtra("Target", &object);
			EventBroadcastManager::getInstance().postEvent(EventNames::ON_REMOVE_RIGID_BODY, params);
		}
	}
	ImGui::EndDisabled();

	if (!canAttachRigidBody)
	{
		ImGui::TextDisabled("Rigid Body is not available for this object type.");
	}

	if (!physicsComponent && !pointLightComponent)
	{
		ImGui::Text("No attached editable components.");
		return;
	}

	if (pointLightComponent && ImGui::CollapsingHeader("Point Light Component", ImGuiTreeNodeFlags_DefaultOpen))
	{
		float intensity = pointLightComponent->getIntensity();
		float range = pointLightComponent->getRange();

		ImGui::BeginDisabled(m_isPlayMode || !object.isEnabled());
		if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 100.0f, "%.2f"))
		{
			pointLightComponent->setIntensity(intensity);
		}
		if (ImGui::DragFloat("Size / Range", &range, 0.05f, 0.1f, 100.0f, "%.2f"))
		{
			pointLightComponent->setRange(range);
			const auto markerScale = range * 0.025f;
			object.getTransform().setScale({ markerScale, markerScale, markerScale });
		}
		ImGui::EndDisabled();

		ImGui::Text("Position controls are in the Transform tab.");
	}

	if (physicsComponent && ImGui::CollapsingHeader("Physics Component", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool physicsEnabled = physicsComponent->isPhysicsEnabled();
		ImGui::BeginDisabled(m_isPlayMode || !object.isEnabled());
		if (ImGui::Checkbox("Physics Enabled", &physicsEnabled))
		{
			Parameters params;
			params.PutExtra("Target", &object);
			params.PutExtra("Enabled", physicsEnabled);
			EventBroadcastManager::getInstance().postEvent(EventNames::ON_SET_PHYSICS_ENABLED, params);
		}
		ImGui::EndDisabled();

		ImGui::Text("Body Type: %s",
			physicsComponent->getBodyType() == PhysicsBodyType::Static ? "Static" :
			physicsComponent->getBodyType() == PhysicsBodyType::Kinematic ? "Kinematic" : "Dynamic");
		ImGui::Text("Mass: %.2f", physicsComponent->getMass());
		ImGui::Text("Gravity: %s", physicsComponent->isUseGravityEnabled() ? "On" : "Off");
	}

}
