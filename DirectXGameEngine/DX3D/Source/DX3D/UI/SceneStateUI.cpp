#include <DX3D/UI/SceneStateUI.h>

#include <imgui.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>

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
					EventBroadcastManager::getInstance().postEvent(EventNames::ON_EDITOR_PLAY_MODE_CHANGED, params);
				}

			}
			// Pause/Resume
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
			// Framestep
			if (ImGui::Button("Frame Step"))
			{

			}
		}
		ImGui::End();
	}
}

dx3d::SceneStateUI::~SceneStateUI()
{
}
