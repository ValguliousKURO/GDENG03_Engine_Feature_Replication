#pragma once
#include <DX3D/All.h>
#include <DX3D/Game/TestUI.h>

class MainGame : public dx3d::Game
{
public:
	explicit MainGame(const dx3d::GameDesc& desc);
protected:
	virtual void onCreate();
	virtual void onUpdate(dx3d::f32 deltaTime);
	virtual void onDrawUi() override; // ADDED: Submits this game's ImGui controls each frame.

private:
	dx3d::UniquePtr<dx3d::TestUI> m_testUi{};
	dx3d::GameObject* m_testObject{}; // ADDED: The centre cube edited by the Transform tab.
};
