#pragma once
#include <DX3D/UI/BaseUI.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <DX3D/Game/GameObject.h>
#include <imgui.h>

namespace dx3d
{

	class HierarchyUI : public BaseUI
	{
		public:
			HierarchyUI(const BaseDesc& desc);
			~HierarchyUI();

			void draw() override;

			void setGameObjectList(const std::unordered_map<size_t, std::vector<UniquePtr<GameObject>>>* list);
			void setObjModelNames(const std::vector<std::string>* modelNames);
		private:
			void drawGameObjectNode(GameObject* obj);
		private:
			bool m_showHierarchy = true;
			bool m_isPlayMode = false;
			const std::unordered_map<size_t, std::vector<UniquePtr<GameObject>>>* m_gameObjects{};
			const std::vector<std::string>* m_objModelNames{};
			GameObject* m_selectedObject{ nullptr };
	};

}
