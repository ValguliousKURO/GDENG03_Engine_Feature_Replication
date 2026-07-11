#pragma once
#include <DX3D/Core/Base.h>

namespace dx3d
{

	class GraphicsDevice;
	class SwapChain;
	class World;
	class GameObject;

	class TestUI : public Base
	{
	public:
		TestUI(const BaseDesc& desc);
		void draw(GameObject& object);

		~TestUI();
	private:
		void render(World& world, GraphicsDevice& graphicsDevice, SwapChain& swapChain);
		void drawGameObjectPanel(World& world);
		void drawTransformInspector(GameObject& object);
		void drawComponentInspector(GameObject& object);

	};

}