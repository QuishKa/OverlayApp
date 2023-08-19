#include "gui.h"
//#include "imgui.h"
//#include "imgui_impl_win32.h"
//#include "imgui_impl_dx11.h"

// declarations
//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
OverLayGui                      OverLayGui::inst;
HINSTANCE                       OverLayGui::hInstance;
WNDCLASSEXW                     OverLayGui::wc;
HWND                            OverLayGui::hwnd;
bool                            OverLayGui::destroyed;
std::unique_ptr<Graphics>       OverLayGui::render = nullptr;
UINT                            OverLayGui::ResizeWidth; 
UINT                            OverLayGui::ResizeHeight;
RECT rect;

bool OverLayGui::OverlayInit()
{
    hInstance = GetModuleHandle(nullptr);
    destroyed = false;
    ResizeHeight = 0;
    ResizeWidth = 0;
    {
        wc = {0};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = inst.WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = nullptr;
        wc.hCursor = nullptr;
        wc.hbrBackground = nullptr;
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = L"Overlay App";
        wc.hIconSm = nullptr;
        RegisterClassExW(&wc);
    }
    rect = { 0, 0, 1366, 768 };
    hwnd = CreateWindowExW(
        0, 
        wc.lpszClassName, 
        L"Overlay App", 
        WS_POPUP, 
        0, 
        0, 
        rect.right - rect.left, 
        rect.bottom - rect.top, 
        nullptr, 
        nullptr, 
        wc.hInstance, 
        nullptr);
    RegisterHotKey(hwnd, 0, MOD_NOREPEAT, VK_TAB);
        //printf("Hotkey %s not binded!\n", ImGui::GetKeyName(ImGuiKey(VK_TAB)));
    //SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), BYTE(100), LWA_ALPHA);
    //MARGINS mrg = {-1, -1, -1, -1};
    //DwmExtendFrameIntoClientArea(hwnd, &mrg);
    
    render = std::make_unique<Graphics>(hwnd);

    if (render == nullptr)
        return 1;

    ShowWindow(hwnd, SW_NORMAL);
    UpdateWindow(hwnd);

    return 0;
}

bool OverLayGui::OverlayStart()
{
    while (!destroyed) 
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                destroyed = true;
        }
        if (destroyed)
            break;
        
        // renders
        if (!IsIconic(hwnd)) 
        {
            if (render->OverlayDraw())
                break;
            
            if (render->OverlayRender())
                break;
        } else Sleep(100);
        //
    }

    return 0;
}

/*
inline void MainGui::StatsRender(ImGuiIO io) 
{
        ImGuiWindowFlags window_flags = 
            ImGuiWindowFlags_NoDecoration | 
            ImGuiWindowFlags_NoDocking | 
            ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoNav;
        
        ImGui::SetNextWindowBgAlpha(0.9f);
        ImGui::Begin("Hello, world!", NULL, window_flags); 
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        if (ImGui::Button("Quit")) 
            ::DestroyWindow(hwnd);
        ImGui::SameLine();
        if (ImGui::Button("Minimize"))
            ::CloseWindow(hwnd);
        ImGui::Separator();
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse Position: <invalid>");
        ImGui::Separator();
        ImGui::End();
} */

LRESULT WINAPI OverLayGui::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_HOTKEY:
        if (lParam == VK_TAB | MOD_NOREPEAT)
        {
            if (IsIconic(hwnd))
                OpenIcon(hwnd);
            else
                CloseWindow(hwnd);
        }
        return 0;
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        destroyed = true;
        return 0;
    case WM_DPICHANGED:
        //if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            SetWindowPos(hwnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

OverLayGui::~OverLayGui()
{
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
}