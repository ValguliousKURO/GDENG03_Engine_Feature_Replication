#pragma once
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Display.h>
#include <DX3D/UI/SceneViewportUI.h>
#include <chrono>
#include <vector>

struct ImGuiContext;

namespace dx3d
{
    class Game
    {
        dx3d_disable_copy_and_move(Game)
    public:
        explicit Game(const GameDesc& desc);
        virtual ~Game();

        virtual World& getWorld() noexcept final;
        virtual WorldRenderer& getWorldRenderer() noexcept final;
        virtual Logger& getLogger() noexcept final;
        
        // MANAGERS
        virtual ResourceManager& getResourceManager() noexcept final;

        virtual MeshFactory& getMeshFactory() noexcept final;

        //run function
        virtual void run() final;

        // add/remove displays
        void addDisplay();
        const std::vector<UniquePtr<Display>>& getDisplays() const noexcept { return m_displays; }

        // add/remove world view
        void addWorldView(std::string name, size_t camera_ID);

        void onRenderSceneViewports();

	protected:
		virtual void onCreate() {}
		virtual void onUpdate(f32 deltaTime) {}
		virtual void onDisplayAdded(Display& display) {}
		virtual void onDrawUi(Display& display) {} // Derived games submit ImGui widgets for the display currently being rendered.
    private:
        void onInternalUpdate();
		void initializeDisplayImGui(Display& display);
		void shutdownImGui();

        UniquePtr<Logger> m_logger{};
        RefPtr<GraphicsDevice> m_graphicsDevice{};
        std::vector<UniquePtr<Display>> m_displays{};   // multiple displays
        std::vector< dx3d::UniquePtr<dx3d::SceneViewportUI>> m_sceneViewportUIs{};
		//manager instances
        UniquePtr<ResourceManager> m_resourceManager{};
		UniquePtr<MeshFactory> m_meshFactory{};
        
        UniquePtr<World> m_world{};
        UniquePtr<WorldRenderer> m_worldRenderer{};
        Rect m_windowSize;
		bool m_isRunning{ true };

		bool m_imguiInitialized{ false }; // Tracks ownership of the shared ImGui context/backends.
		ImGuiContext* m_imguiContext{};
		ui32 m_pendingDisplayAdditions{};

        std::chrono::steady_clock::time_point m_previousTime{};

        SceneState m_currentState = SceneState::EDIT;
    };
}
