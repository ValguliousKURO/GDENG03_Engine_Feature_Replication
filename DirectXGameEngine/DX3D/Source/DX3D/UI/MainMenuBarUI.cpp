#include <DX3D/UI/MainMenuBarUI.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <DX3D/EventBroadcasting/Parameters.h>

// ImGui's DragFloat3 range uses FLT_MAX.
#include <cfloat>
#include <algorithm>
#include <filesystem>
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
			if (ImGui::MenuItem("Save Level", "Ctrl+S"))
			{
				EventBroadcastManager::getInstance().postEvent(EventNames::ON_SCENE_SAVE);
			}
			if (ImGui::MenuItem("Save Level As New"))
			{
				EventBroadcastManager::getInstance().postEvent(EventNames::ON_SCENE_SAVE_AS_NEW);
			}
			if (ImGui::BeginMenu("Load Level"))
			{
				const auto sceneFiles = getAvailableSceneFiles();
				if (sceneFiles.empty())
				{
					ImGui::MenuItem("No .level files found", nullptr, false, false);
				}
				else
				{
					for (const auto& scenePath : sceneFiles)
					{
						const auto sceneName = scenePath.filename().string();
						if (ImGui::MenuItem(sceneName.c_str()))
						{
							Parameters params;
							params.PutExtra("Path", scenePath.string());
							EventBroadcastManager::getInstance().postEvent(EventNames::ON_SCENE_LOAD, params);
						}
					}
				}
				ImGui::EndMenu();
			}
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
		if (ImGui::BeginMenu("Editor"))
		{
			if (ImGui::MenuItem("Undo", "Ctrl+Z"))
			{
				EventBroadcastManager::getInstance().postEvent(EventNames::ON_EDITOR_UNDO);
			}
			if (ImGui::MenuItem("Redo", "Ctrl+Y"))
			{
				EventBroadcastManager::getInstance().postEvent(EventNames::ON_EDITOR_REDO);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Edit Mode"))
			{
				Parameters params;
				params.PutExtra("IsPlayMode", false);
				EventBroadcastManager::getInstance().postEvent(EventNames::ON_EDITOR_PLAY_MODE_CHANGED, params);
			}
			if (ImGui::MenuItem("Play Mode"))
			{
				Parameters params;
				params.PutExtra("IsPlayMode", true);
				EventBroadcastManager::getInstance().postEvent(EventNames::ON_EDITOR_PLAY_MODE_CHANGED, params);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

std::filesystem::path dx3d::MainMenuBarUI::getScenesDirectory() const
{
	return std::filesystem::current_path() / "Assets" / "Scenes";
}

std::vector<std::filesystem::path> dx3d::MainMenuBarUI::getAvailableSceneFiles() const
{
	namespace fs = std::filesystem;

	std::vector<fs::path> sceneFiles;
	std::error_code error;
	const auto scenesDirectory = getScenesDirectory();

	if (!fs::exists(scenesDirectory, error) || !fs::is_directory(scenesDirectory, error))
	{
		return sceneFiles;
	}

	for (const auto& entry : fs::directory_iterator(scenesDirectory, error))
	{
		if (error) return sceneFiles;
		if (!entry.is_regular_file(error)) continue;

		auto path = entry.path();
		const auto extension = path.extension().string();
		if (extension == ".level" || extension == ".LEVEL" || extension == ".json" || extension == ".JSON")
		{
			sceneFiles.push_back(path);
		}
	}

	std::sort(sceneFiles.begin(), sceneFiles.end());
	return sceneFiles;
}
