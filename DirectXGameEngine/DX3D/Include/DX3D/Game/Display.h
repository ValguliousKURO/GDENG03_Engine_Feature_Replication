#pragma once
#include <DX3D/Window/Window.h>
#include <DX3D/Input/InputSystem.h>

namespace dx3d
{
    class Display final : public Window
    {
    public:
        explicit Display(const DisplayDesc& desc);

        SwapChain& getSwapChain() noexcept;
        InputSystem& getInputSystem() noexcept { return *m_inputSystem; }

    private:
        RefPtr<SwapChain> m_swapChain{};
        UniquePtr<InputSystem> m_inputSystem{};
    };
}
