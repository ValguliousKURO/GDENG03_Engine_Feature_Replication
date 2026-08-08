#pragma once

#include <DX3D/UI/BaseUI.h>

namespace dx3d
{

	class SceneStateUI : public BaseUI
	{
	public:
		SceneStateUI(const BaseDesc& desc);
		void draw() override;

		~SceneStateUI();

	private:
		bool m_showUI = true;
		std::string m_currentStateLabel = "Play";
		std::string m_currentPauseLabel = "Pause";
	};
}