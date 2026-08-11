#include <DX3D/UI/SceneStateUI.h>

#include <imgui.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>

#include <DX3D/Core/DebugLogManager.h>

dx3d::SceneStateUI::SceneStateUI(const BaseDesc& desc) : BaseUI(desc)
{
}

void dx3d::SceneStateUI::draw()
{
	if (m_showUI)
	{
		if (ImGui::Begin("Scene State", &m_showUI))
		{
			// Play/Stop
			if (ImGui::Button(m_currentStateLabel.c_str()))
			{
				Parameters params;
				if (m_currentStateLabel == "Play")
				{
					params.PutExtra("IsPlayMode", true);
					m_currentStateLabel = "Stop";
					EventBroadcastManager::getInstance().postEvent(EventNames::ON_EDITOR_PLAY_MODE_CHANGED, params);
				}
				else
				{
					params.PutExtra("IsPlayMode", false);
					m_currentStateLabel = "Play";
					m_currentPauseLabel = "Pause";
					EventBroadcastManager::getInstance().postEvent(EventNames::ON_EDITOR_PLAY_MODE_CHANGED, params);
				}

			}
			// Pause/Resume
			ImGui::BeginDisabled(m_currentStateLabel != "Stop"); // Disable if not paused
			if (ImGui::Button(m_currentPauseLabel.c_str()))
			{
				Parameters params;
				if (m_currentPauseLabel == "Pause")
				{
					params.PutExtra("IsPauseState", true);
					m_currentPauseLabel = "Unpause";
					EventBroadcastManager::getInstance().postEvent(EventNames::ON_SCENE_PAUSE_STATE_CHANGED, params);
				}
				else
				{
					params.PutExtra("IsPauseState", false);
					m_currentPauseLabel = "Pause";
					EventBroadcastManager::getInstance().postEvent(EventNames::ON_SCENE_PAUSE_STATE_CHANGED, params);
				}
			}
			ImGui::EndDisabled();
			// Framestep
			ImGui::BeginDisabled(m_currentPauseLabel != "Unpause"); // Disable if not paused
			if (ImGui::Button("Frame Step"))
			{
				EventBroadcastManager::getInstance().postEvent(EventNames::ON_SCENE_FRAME_STEP);
				DebugLogManager::getInstance().customLog("Frame Step Triggered.");
			}
			ImGui::EndDisabled();
		
		

			
		}
		ImGui::End();
	}
}

dx3d::SceneStateUI::~SceneStateUI()
{
}
