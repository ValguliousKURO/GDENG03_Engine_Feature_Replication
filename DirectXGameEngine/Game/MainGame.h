#pragma once
#include <DX3D/All.h>
#include <DX3D/UI/InspectorUI.h>
#include <DX3D/UI/MainMenuBarUI.h>
#include <DX3D/Graphics/Mesh/Mesh.h>
#include <DX3D/Resource/MaterialResource.h>
#include <unordered_map>
#include <functional>

class MainGame : public dx3d::Game
{
public:
	explicit MainGame(const dx3d::GameDesc& desc);

	void onNewDisplay(dx3d::Display& display);
	void onNewWorldView(std::string name);

protected:
	virtual void onCreate();
	virtual void onUpdate(dx3d::f32 deltaTime);
	virtual void onDisplayAdded(dx3d::Display& display) override;
	virtual void onDrawUi(dx3d::Display& display) override; // ADDED: Submits this game's ImGui controls for one display each frame.

private:
	struct EditorCommand
	{
		std::function<void()> undo;
		std::function<void()> redo;
	};

	void registerEditorEvents();
	void executeEditorCommand(EditorCommand command);
	void undoEditorCommand();
	void redoEditorCommand();
	void setPlayMode(bool isPlayMode);

	dx3d::GameObject* spawnEditorObject(const std::string& type);
	void selectGameObject(dx3d::GameObject* object);

	std::vector< dx3d::UniquePtr<dx3d::BaseUI>> m_UIs{};
	std::unordered_map<dx3d::ui32, dx3d::UniquePtr<dx3d::InspectorUI>> m_InspectorUIs{};
	dx3d::GameObject* m_testObject{}; // ADDED: The centre cube edited by the Transform tab.
	dx3d::RefPtr<dx3d::Mesh> m_spawnCubeMesh{};
	dx3d::RefPtr<dx3d::Mesh> m_spawnSphereMesh{};
	dx3d::RefPtr<dx3d::Mesh> m_spawnCapsuleMesh{};
	dx3d::RefPtr<dx3d::Mesh> m_spawnCylinderMesh{};
	dx3d::RefPtr<dx3d::Mesh> m_spawnPlaneMesh{};
	dx3d::RefPtr<dx3d::MaterialResource> m_spawnMaterial{};
	std::vector<EditorCommand> m_undoStack{};
	std::vector<EditorCommand> m_redoStack{};
	std::vector<std::string> m_availableObjModels{};
	bool m_isPlayMode{ false };
	std::unordered_map<std::string, size_t> m_spawnedObjectCounters{};
	static constexpr size_t MaxUndoCommands = 20;
};
