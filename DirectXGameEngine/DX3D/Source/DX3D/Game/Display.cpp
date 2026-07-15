#include <DX3D/Game/Display.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <string>

dx3d::Display::Display(const DisplayDesc& desc): Window(desc.window)
{
	m_swapChain = desc.graphicsDevice.createSwapChain({ m_handle, m_size });

	m_inputSystem = std::make_unique<InputSystem>(InputSystemDesc{ m_logger });
	m_inputSystem->setCursorLockArea(getClientAreaInScreenSpace());

	dx3d::EventBroadcastManager::getInstance().addObserver(
		dx3d::EventNames::LIT_MODE_TOGGLE + "_" + std::to_string(getID()),
		[this]() { this->setRenderMode(RenderMode::Lit); }
	);
	dx3d::EventBroadcastManager::getInstance().addObserver(
		dx3d::EventNames::WIREFRAME_MODE_TOGGLE + "_" + std::to_string(getID()),
		[this]() { this->setRenderMode(RenderMode::Wireframe); }
	);
	dx3d::EventBroadcastManager::getInstance().addObserver(
		dx3d::EventNames::WIREFRAME_TOGGLE + "_" + std::to_string(getID()),
		[this]() { this->setRenderMode(this->getRenderMode() == RenderMode::Lit ? RenderMode::Wireframe : RenderMode::Lit); }
	);
}

dx3d::Display::~Display()
{
	dx3d::EventBroadcastManager::getInstance().RemoveObserver(dx3d::EventNames::LIT_MODE_TOGGLE + "_" + std::to_string(getID()));
	dx3d::EventBroadcastManager::getInstance().RemoveObserver(dx3d::EventNames::WIREFRAME_MODE_TOGGLE + "_" + std::to_string(getID()));
	dx3d::EventBroadcastManager::getInstance().RemoveObserver(dx3d::EventNames::WIREFRAME_TOGGLE + "_" + std::to_string(getID()));
}

dx3d::SwapChain& dx3d::Display::getSwapChain() noexcept
{
	return *m_swapChain;
}
