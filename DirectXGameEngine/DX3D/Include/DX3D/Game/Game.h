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
        void addDisplay(const DisplayDesc& desc);
        const std::vector<UniquePtr<Display>>& getDisplays() const noexcept { return m_displays; }

	protected:
		virtual void onCreate() {}
		virtual void onUpdate(f32 deltaTime) {}
		virtual void onDrawUi() {} // ADDED: Derived games submit their ImGui widgets here every frame.
    private:
        void onInternalUpdate();

        UniquePtr<Logger> m_logger{};
        RefPtr<GraphicsDevice> m_graphicsDevice{};
        std::vector<UniquePtr<Display>> m_displays{};   // multiple displays
        UniquePtr<ResourceManager> m_resourceManager{};
        UniquePtr<World> m_world{};
        UniquePtr<WorldRenderer> m_worldRenderer{};
		bool m_isRunning{ true };
		bool m_imguiInitialized{ false }; // ADDED: Tracks ownership of the shared ImGui context and backends.
        std::chrono::steady_clock::time_point m_previousTime{};
    };
}
