#include <DX3D/Window/Window.h>
#include <Windows.h>
#include <stdexcept>
#include <atomic>

//include window impl
#include <imgui.h>
#include <imgui_impl_win32.h>

// backend header implement for ui
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


static std::atomic<uint32_t> g_windowCounter{ 0 };

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    dx3d::Window* pThis = nullptr;

    if (msg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lparam);
        pThis = reinterpret_cast<dx3d::Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else
    {
        pThis = reinterpret_cast<dx3d::Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis && pThis->getImGuiContext())
    {
        ImGui::SetCurrentContext(pThis->getImGuiContext());
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
            return true;
    }

    if (pThis)
        return pThis->handleMessage(msg, wparam, lparam);

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

dx3d::Window::Window(const WindowDesc& desc)
    : Base(desc.base), m_size(desc.size)
{
    m_id = ++g_windowCounter;

    auto registerWindowClassFunction = []()
        {
            WNDCLASSEX wc{};
            wc.cbSize = sizeof(WNDCLASSEX);
            wc.lpszClassName = L"DX3DWindow";
            wc.lpfnWndProc = &WindowProcedure;
            wc.hInstance = GetModuleHandle(nullptr);
            wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            return RegisterClassEx(&wc);
        };

    static const auto windowClassId = std::invoke(registerWindowClassFunction);

    if (!windowClassId)
    {
        DX3DLogThrowError("RegisterClassEx failed!");
    }

    RECT rc{ 0,0,m_size.width,m_size.height };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

    m_handle = CreateWindowEx(
        0,
        MAKEINTATOM(windowClassId),
        L"C++ 3D Engine | Based on PardCode C++ 3D Game Tutorial Series",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, GetModuleHandle(nullptr),
        this // Pass pointer to WM_CREATE
    );

    if (!m_handle)
    {
        DX3DLogThrowError("CreateWindowEx failed!");
    }

    ShowWindow(static_cast<HWND>(m_handle), SW_SHOW);
}

dx3d::Rect dx3d::Window::getClientAreaInScreenSpace()
{
    auto hwnd = static_cast<HWND>(m_handle);

    RECT client{};
    GetClientRect(hwnd, &client);

    POINT topLeft{ client.left, client.top };
    POINT bottomRight{ client.right, client.bottom };
    ClientToScreen(hwnd, &topLeft);
    ClientToScreen(hwnd, &bottomRight);

    return {
        topLeft.x,
        topLeft.y,
        bottomRight.x - topLeft.x,
        bottomRight.y - topLeft.y
    };
}

dx3d::Window::~Window()
{
    if (m_handle)
        DestroyWindow(static_cast<HWND>(m_handle));
}

// Default message handler
LRESULT dx3d::Window::handleMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{

	switch (msg)
    {
    case WM_CLOSE:
        m_isClosed = true;
        if (m_handle)
        {
            auto handle = static_cast<HWND>(m_handle);
            m_handle = nullptr;
            DestroyWindow(handle);
        }
        return 0;

    case WM_DESTROY:
        m_isClosed = true;
        m_handle = nullptr;
        break;

    case WM_SETFOCUS:
        m_hasFocus = true;
        break;

    case WM_KILLFOCUS:
        m_hasFocus = false;
        break;

    default:
        return DefWindowProc(static_cast<HWND>(m_handle), msg, wparam, lparam);
    }
    return 0;
}

