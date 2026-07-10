#pragma once
#include <DX3D/Window/Window.h>
#include <DX3D/Input/InputSystem.h>
#include <DX3D/Component/CameraComponent.h>

namespace dx3d
{
    class Display final : public Window
    {
    public:
        explicit Display(const DisplayDesc& desc);

        SwapChain& getSwapChain() noexcept;
        InputSystem& getInputSystem() noexcept { return *m_inputSystem; }

        void setCamera(CameraComponent* cam) { m_camera = cam; }
        CameraComponent* getCamera() const noexcept { return m_camera; }

    private:
        RefPtr<SwapChain> m_swapChain{};
        UniquePtr<InputSystem> m_inputSystem{};
        CameraComponent* m_camera{};
    };

}
