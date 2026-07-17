#pragma once
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Core.h>
#include <DX3D/Game/Display.h>
#include <chrono>
#include <vector>

namespace dx3d
{
    class Game
    {
        dx3d_disable_copy_and_move(Game)
    public:
        explicit Game(const GameDesc& desc);
        virtual ~Game();

        virtual World& getWorld() noexcept final;
        virtual Logger& getLogger() noexcept final;
        virtual ResourceManager& getResourceManager() noexcept final;
        virtual void run() final;

        // New: add/remove displays
        void addDisplay();
        const std::vector<UniquePtr<Display>>& getDisplays() const noexcept { return m_displays; }

	protected:
		virtual void onCreate() {}
		virtual void onUpdate(f32 deltaTime) {}
		virtual void onDisplayAdded(Display& display) {}
		virtual void onDrawUi(Display& display) {} // ADDED: Derived games submit ImGui widgets for the display currently being rendered.
    private:
        void onInternalUpdate();
		void initializeDisplayImGui(Display& display);
		void shutdownDisplayImGui(Display& display);

        UniquePtr<Logger> m_logger{};
        RefPtr<GraphicsDevice> m_graphicsDevice{};
        std::vector<UniquePtr<Display>> m_displays{};   // multiple displays
        UniquePtr<ResourceManager> m_resourceManager{};
        UniquePtr<World> m_world{};
        UniquePtr<WorldRenderer> m_worldRenderer{};
        Rect m_windowSize;
		bool m_isRunning{ true };

		bool m_imguiInitialized{ false }; // ADDED: Tracks whether display ImGui contexts may be shut down.
		ui32 m_pendingDisplayAdditions{};

        std::chrono::steady_clock::time_point m_previousTime{};
    };
}
