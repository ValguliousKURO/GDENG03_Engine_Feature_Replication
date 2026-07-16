#pragma once
#include <DX3D/Core/Base.h>

namespace dx3d
{

	class GraphicsDevice;
	class SwapChain;
	class World;
	class GameObject;

	class BaseUI : public Base
	{
	public:
		BaseUI(const BaseDesc& desc);
		virtual void draw() = 0;

		~BaseUI();

	};

}