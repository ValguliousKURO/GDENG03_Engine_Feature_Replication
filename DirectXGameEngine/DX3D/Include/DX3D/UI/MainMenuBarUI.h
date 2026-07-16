#pragma once
#include <DX3D/UI/BaseUI.h>

namespace dx3d
{
	class MainMenuBarUI : BaseUI
	{
	public:
		MainMenuBarUI(const BaseDesc& desc);
		~MainMenuBarUI();

		void draw() override;
	};
}
