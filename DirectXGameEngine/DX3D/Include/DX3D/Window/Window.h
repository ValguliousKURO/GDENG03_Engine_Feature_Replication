#pragma once
#include <DX3D/Core/Base.h>
#include <DX3D/Core/Common.h>
#include <Windows.h>

namespace dx3d
{
    class Window : public Base
    {
    public:
        explicit Window(const WindowDesc& desc);
        virtual ~Window() override;

        dx3d::Rect getClientAreaInScreenSpace();
        uint32_t getID() const { return m_id; }
        void* getHandle() const { return m_handle; }
        const Rect& getSize() const { return m_size; }

        virtual LRESULT handleMessage(UINT msg, WPARAM wparam, LPARAM lparam);
        bool hasFocus() const noexcept { return m_hasFocus; }

    protected:
        uint32_t m_id{};  // Unique window identifier
        void* m_handle{};
        Rect m_size{};
        bool m_hasFocus{ false };

    };
}
