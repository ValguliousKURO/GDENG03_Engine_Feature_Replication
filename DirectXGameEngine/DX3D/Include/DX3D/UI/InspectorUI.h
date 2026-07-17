#pragma once
#include <DX3D/Core/Base.h>
#include <DX3D/UI/BaseUI.h>
#include <vector>

namespace dx3d
{

	class InspectorUI : public BaseUI
	{
	public:
		InspectorUI(const BaseDesc& desc);
		void draw(GameObject& object, Display& display);
		void draw() override;

		~InspectorUI();
	private:
		void render(World& world, GraphicsDevice& graphicsDevice, SwapChain& swapChain);
		void drawGameObjectPanel(World& world);
		void drawTransformInspector(GameObject& object);
		void drawComponentInspector(GameObject& object);
		void drawViewportPanel(Display& display);

	};

}
