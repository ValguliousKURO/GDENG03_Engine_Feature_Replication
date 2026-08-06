#pragma once
#include <DX3D/UI/BaseUI.h>
#include <unordered_map>
#include <DX3D/Game/GameObject.h>
#include <imgui.h>
#include <string>
#include <utility>
#include <vector>

namespace dx3d
{

	class DebugWindowUI : public BaseUI
	{
	public:
		DebugWindowUI(const BaseDesc& desc);
		~DebugWindowUI();

		void draw() override;
		void addLog(std::string entry);


	private:
		bool m_ShowDebug = true;
		bool m_ScrollToBottom = false;
		std::vector<std::string> m_LogEntries;

	};

}
