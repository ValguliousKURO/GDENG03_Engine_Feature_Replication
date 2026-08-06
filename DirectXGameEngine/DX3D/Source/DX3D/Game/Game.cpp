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

// MANAGERS
#include <DX3D/Resource/ResourceManager.h>
#include <DX3D/Graphics/Mesh/MeshFactory.h>

#include <DX3D/EventBroadcasting/EventBroadcastManager.h>
#include <DX3D/EventBroadcasting/EventNames.h>
#include <DX3D/Graphics/RenderSystem/SwapChain/SwapChain.h>

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

	auto context = SystemContext{ *m_graphicsDevice };
	m_resourceManager = std::make_unique<ResourceManager>(ResourceManagerDesc{ {*m_logger}, context });
	m_meshFactory = std::make_unique<MeshFactory>(MeshFactoryDesc{ {*m_logger} });

	m_world = std::make_unique<World>(WorldDesc{ BaseDesc{*m_logger}, GameContext{m_displays.front()->getInputSystem(), *m_resourceManager, *m_graphicsDevice}});

	m_worldRenderer = std::make_unique<WorldRenderer>(WorldRendererDesc{ {*m_logger}, *m_graphicsDevice });

	m_imguiInitialized = true;

	// Events
	EventBroadcastManager::getInstance().addObserver(
		EventNames::ON_WINDOW_NEW, 
		[this]() { ++m_pendingDisplayAdditions; });

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport

	DX3DLogInfo("Game Initialized!");

	/// testing add Obj mesh data  via meshfactory here
	m_meshFactory->loadAll();
}

dx3d::World& dx3d::Game::getWorld() noexcept
{
	return *m_world;
}

dx3d::WorldRenderer& dx3d::Game::getWorldRenderer() noexcept
{
	return *m_worldRenderer;
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

dx3d::MeshFactory& dx3d::Game::getMeshFactory() noexcept
{
	return *m_meshFactory;
}

void dx3d::Game::onInternalUpdate()
{
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<f32> delta = currentTime - m_previousTime;
	m_previousTime = currentTime;
	auto deltaTime = delta.count();
	ImGuiIO& io = ImGui::GetIO();

	// Update each display's input system separately
	for (auto& display : m_displays)
	{
		if (display->hasFocus())
		{
			display->getInputSystem().update();
		}
	}

	// TODO: Update each viewport's input system seperately
	//

	//

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
		ImGui::DockSpaceOverViewport();
		onRenderSceneViewports();
		onDrawUi(*display);
		ImGui::Render();

		m_worldRenderer->renderForDisplay(*m_world, *display, deltaTime, ImGui::GetDrawData());


		// Handle multiple viewports
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

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

void dx3d::Game::addWorldView(std::string name, size_t camera_ID)
{
	std::unique_ptr<dx3d::SceneViewportUI> new_SceneViewportUI = std::make_unique<dx3d::SceneViewportUI>(
		dx3d::BaseDesc{ getLogger() }, 
		getWorld(), 
		getWorldRenderer()
	);
	new_SceneViewportUI->setName(name);
	new_SceneViewportUI->setID(camera_ID);
	m_sceneViewportUIs.push_back(std::move(new_SceneViewportUI));
}

void dx3d::Game::onRenderSceneViewports()
{
	for (auto& vp : m_sceneViewportUIs)
	{
		vp->draw();
	}
}

void dx3d::Game::initializeDisplayImGui(Display& display)
{
	auto* context = ImGui::CreateContext();
	display.setImGuiContext(context);

	ImGui::SetCurrentContext(context);
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(display.getHandle());
	ImGui_ImplDX11_Init(m_graphicsDevice->getNativeDevice(), m_graphicsDevice->getNativeContext());

	// Force ImGui to create its DX11 device objects for this context
	ImGui_ImplDX11_CreateDeviceObjects();

	// Set DisplaySize
	ImGuiIO& io = ImGui::GetIO();
	auto size = display.getSwapChain().getSize();
	io.DisplaySize = ImVec2((float)size.width, (float)size.height);
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
