#pragma once
#include <DX3D/Core/Base.h>
#include <vector>

namespace dx3d
{

	class GraphicsDevice;
	class SwapChain;
	class World;
	class GameObject;
	class Display;

	class TestUI : public Base
	{
	public:
		TestUI(const BaseDesc& desc);
		void draw(GameObject& object, const std::vector<std::unique_ptr<Display>>& displays);

		~TestUI();
	private:
		void render(World& world, GraphicsDevice& graphicsDevice, SwapChain& swapChain);
		void drawGameObjectPanel(World& world);
		void drawTransformInspector(GameObject& object);
		void drawComponentInspector(GameObject& object);
		void drawViewportPanel(const std::vector<std::unique_ptr<Display>>& displays);

	};

}