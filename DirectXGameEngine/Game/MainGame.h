#pragma once
#include <DX3D/All.h>
#include <DX3D/UI/InspectorUI.h>
#include <DX3D/UI/MainMenuBarUI.h>
#include <unordered_map>

class MainGame : public dx3d::Game
{
public:
	explicit MainGame(const dx3d::GameDesc& desc);

	void onNewDisplay(dx3d::Display& display);

protected:
	virtual void onCreate();
	virtual void onUpdate(dx3d::f32 deltaTime);
	virtual void onDisplayAdded(dx3d::Display& display) override;
	virtual void onDrawUi(dx3d::Display& display) override; // ADDED: Submits this game's ImGui controls for one display each frame.

private:
	std::unordered_map<dx3d::ui32, dx3d::UniquePtr<dx3d::InspectorUI>> m_InspectorUIs{};
	dx3d::UniquePtr<dx3d::MainMenuBarUI> m_MainMenuBarUI{};
	dx3d::GameObject* m_testObject{}; // ADDED: The centre cube edited by the Transform tab.
};
