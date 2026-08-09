#pragma once
#include <DX3D/UI/BaseUI.h>
#include <filesystem>
#include <vector>

namespace dx3d
{
	class MainMenuBarUI : public BaseUI
	{
	public:
		MainMenuBarUI(const BaseDesc& desc);
		~MainMenuBarUI();

		void draw() override;

	private:
		std::filesystem::path getScenesDirectory() const;
		std::vector<std::filesystem::path> getAvailableSceneFiles() const;
	};
}
