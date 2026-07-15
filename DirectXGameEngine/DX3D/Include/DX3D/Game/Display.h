#pragma once
#include <DX3D/Window/Window.h>
#include <DX3D/Input/InputSystem.h>
#include <DX3D/Component/CameraComponent.h>

namespace dx3d
{
    class Display final : public Window
    {
    public:
        enum class RenderMode
        {
            Lit,
            Wireframe
        };

        explicit Display(const DisplayDesc& desc);
        virtual ~Display() override;

        SwapChain& getSwapChain() noexcept;
        InputSystem& getInputSystem() noexcept { return *m_inputSystem; }

        void setCamera(CameraComponent* cam) { m_camera = cam; }
        CameraComponent* getCamera() const noexcept { return m_camera; }

        void setRenderMode(RenderMode mode) noexcept { m_renderMode = mode; }
        RenderMode getRenderMode() const noexcept { return m_renderMode; }

    private:
        RefPtr<SwapChain> m_swapChain{};
        UniquePtr<InputSystem> m_inputSystem{};
        CameraComponent* m_camera{};
        RenderMode m_renderMode{ RenderMode::Lit };
    };

}
