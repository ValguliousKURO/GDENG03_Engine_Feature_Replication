#include <DX3D/Game/Game.h>
#include <DX3D/Window/Window.h>
#include <DX3D/Graphics/GraphicsDevice.h>
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Logger.h>
#include <DX3D/Input/InputSystem.h>
#include <DX3D/Game/Display.h>
#include <DX3D/Game/World.h>
#include <DX3D/Game/GameObject.h>
#include <DX3D/Game/WorldRenderer.h>
#include <DX3D/Resource/ResourceManager.h>

// ADDED: Engine-level setup for Dear ImGui's Win32 and DirectX 11 backends.
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

dx3d::Game::Game(const GameDesc& desc)
{
	m_logger = std::make_unique<Logger>(desc.logLevel);

	DX3DLogInfo("GDENG03");
	DX3DLogInfo("-----------------");

	m_graphicsDevice = std::make_shared<GraphicsDevice>(GraphicsDeviceDesc{ *m_logger });

	// Create the primary display
	addDisplay(DisplayDesc{ {*m_logger, desc.windowSize}, *m_graphicsDevice });
	//secondary display
	addDisplay(DisplayDesc{ {*m_logger, desc.windowSize}, *m_graphicsDevice });
	//third display
	addDisplay(DisplayDesc{ {*m_logger, desc.windowSize}, *m_graphicsDevice });

	auto context = SystemContext{ *m_graphicsDevice };
	m_resourceManager = std::make_unique<ResourceManager>(ResourceManagerDesc{ {*m_logger}, context });

	// Set world for each display
	for (auto& display : m_displays)
	{
		m_world = std::make_unique<World>(WorldDesc{ BaseDesc{*m_logger}, GameContext{display->getInputSystem(), *m_resourceManager, *m_graphicsDevice}});
	}

	m_worldRenderer = std::make_unique<WorldRenderer>(WorldRendererDesc{ {*m_logger}, *m_graphicsDevice });

	// ADDED: One ImGui context is shared by the game and drawn on the first display.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(m_displays.front()->getHandle());
	ImGui_ImplDX11_Init(m_graphicsDevice->getNativeDevice(), m_graphicsDevice->getNativeContext());
	m_imguiInitialized = true;

	DX3DLogInfo("Game Initialized!");
}

dx3d::World& dx3d::Game::getWorld() noexcept
{
	return *m_world;
}

dx3d::Logger& dx3d::Game::getLogger() noexcept
{
	return *m_logger;
}

dx3d::Game::~Game()
{
	if (m_imguiInitialized) // for disabling ui on shutdown
	{
		ImGui_ImplDX11_Shutdown(); 
		ImGui_ImplWin32_Shutdown(); 
		ImGui::DestroyContext(); 
	}

	DX3DLogInfo("Game is shutting down...");
}

dx3d::ResourceManager& dx3d::Game::getResourceManager() noexcept
{
	return *m_resourceManager;
}

void dx3d::Game::onInternalUpdate()
{
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<f32> delta = currentTime - m_previousTime;
	m_previousTime = currentTime;
	auto deltaTime = delta.count();

	// Update each display痴 input system separately
	for (auto& display : m_displays)
	{
		if (display->hasFocus())
		{
			display->getInputSystem().update();
		}
	}

	onUpdate(deltaTime);
	m_world->update(deltaTime);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	onDrawUi();
	ImGui::Render();

	// Render the same completed ImGui frame into every display's swap-chain.
	// The renderer binds each display's back buffer before drawing the UI.
	m_worldRenderer->renderForDisplays(*m_world, m_displays, deltaTime, ImGui::GetDrawData());
}

void dx3d::Game::addDisplay(const DisplayDesc& desc)
{
	auto display = std::make_unique<Display>(desc);
	m_displays.push_back(std::move(display));
}
