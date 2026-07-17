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
#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>

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
	IMGUI_CHECKVERSION();

	m_windowSize = desc.windowSize;
	// Adding intial displays
	addDisplay();
	addDisplay();
	addDisplay();

	auto context = SystemContext{ *m_graphicsDevice };
	m_resourceManager = std::make_unique<ResourceManager>(ResourceManagerDesc{ {*m_logger}, context });

	m_world = std::make_unique<World>(WorldDesc{ BaseDesc{*m_logger}, GameContext{m_displays.front()->getInputSystem(), *m_resourceManager, *m_graphicsDevice}});

	m_worldRenderer = std::make_unique<WorldRenderer>(WorldRendererDesc{ {*m_logger}, *m_graphicsDevice });

	m_imguiInitialized = true;

	// Events
	EventBroadcastManager::getInstance().addObserver(
		EventNames::ON_WINDOW_NEW, 
		[this]() { ++m_pendingDisplayAdditions; });

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
		for (auto& display : m_displays)
		{
			shutdownDisplayImGui(*display);
		}
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

	auto openDisplayCount = 0u;
	for (auto& display : m_displays)
	{
		if (display->isClosed())
		{
			shutdownDisplayImGui(*display);
			continue;
		}

		auto* hwnd = static_cast<HWND>(display->getHandle());
		if (!hwnd || IsIconic(hwnd))
			continue;

		++openDisplayCount;

		ImGui::SetCurrentContext(display->getImGuiContext());
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		onDrawUi(*display);
		ImGui::Render();

		m_worldRenderer->renderForDisplay(*m_world, *display, deltaTime, ImGui::GetDrawData());
	}

	while (m_pendingDisplayAdditions > 0)
	{
		addDisplay();
		onDisplayAdded(*m_displays.back());
		--m_pendingDisplayAdditions;
	}

	if (openDisplayCount == 0 && m_pendingDisplayAdditions == 0)
	{
		m_isRunning = false;
	}
}

void dx3d::Game::addDisplay()
{
	auto display = std::make_unique<Display>(DisplayDesc{ {*m_logger, m_windowSize}, *m_graphicsDevice });
	initializeDisplayImGui(*display);
	m_displays.push_back(std::move(display));
}

void dx3d::Game::initializeDisplayImGui(Display& display)
{
	auto* context = ImGui::CreateContext();
	display.setImGuiContext(context);

	ImGui::SetCurrentContext(context);
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(display.getHandle());
	ImGui_ImplDX11_Init(m_graphicsDevice->getNativeDevice(), m_graphicsDevice->getNativeContext());
}

void dx3d::Game::shutdownDisplayImGui(Display& display)
{
	auto* context = display.getImGuiContext();
	if (!context)
		return;

	ImGui::SetCurrentContext(context);
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext(context);
	display.setImGuiContext(nullptr);
}
