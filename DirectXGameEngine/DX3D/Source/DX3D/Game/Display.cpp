#include <DX3D/Game/Display.h>
#include <DX3D/Graphics/GraphicsDevice.h>

dx3d::Display::Display(const DisplayDesc& desc): Window(desc.window)
{
	m_swapChain = desc.graphicsDevice.createSwapChain({ m_handle, m_size });

	m_inputSystem = std::make_unique<InputSystem>(InputSystemDesc{ m_logger });
	m_inputSystem->setCursorLockArea(getClientAreaInScreenSpace());
}

dx3d::SwapChain& dx3d::Display::getSwapChain() noexcept
{
	return *m_swapChain;
}
