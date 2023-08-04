#include "gui.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <gdiplus.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <windows.h>
#include <iostream>

// declarations
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
MainGui                         MainGui::inst;
bool                            MainGui::destroyed;
ID3D11Device*                   MainGui::g_pd3dDevice;
ID3D11DeviceContext*            MainGui::g_pd3dDeviceContext;
IDXGISwapChain*                 MainGui::g_pSwapChain;
UINT                            MainGui::g_ResizeWidth;
UINT                            MainGui::g_ResizeHeight;
ID3D11RenderTargetView*         MainGui::g_mainRenderTargetView;
WNDCLASSEXW                     MainGui::wc;
HWND                            MainGui::hwnd;
ULONG_PTR                       MainGui::token;
Gdiplus::Bitmap*                MainGui::bmp = nullptr;
ID3D11ShaderResourceView* out_srv;
bool bitmapset = false;

bool MainGui::MainGuiInit()
{
    //inst = MainGui();
    g_pd3dDevice = nullptr;
    g_pd3dDeviceContext = nullptr;
    g_pSwapChain = nullptr;
    g_ResizeWidth = 0,
    g_ResizeHeight = 0;
    g_mainRenderTargetView = nullptr;

    wc = WNDCLASSEXW{ sizeof(wc), CS_HREDRAW | CS_VREDRAW, inst.WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    hwnd = ::CreateWindowExW(WS_EX_TRANSPARENT, wc.lpszClassName, L"Overlay App", WS_POPUP, 0, 0, 1366, 768, nullptr, nullptr, wc.hInstance, nullptr);
    if (!RegisterHotKey(hwnd, 0, MOD_NOREPEAT, VK_TAB))
        printf("Hotkey %s not binded!\n", ImGui::GetKeyName(ImGuiKey(VK_TAB)));
    //SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
    MARGINS mrg = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &mrg);

    if (!inst.CreateDeviceD3D(hwnd))
    {
        inst.CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    io.ConfigViewportsNoDefaultParent = false;

    ImGui::StyleColorsDark();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&token, &input, 0);

    while (!destroyed) 
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                destroyed = true;
        }
        if (destroyed)
            break;
        
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            inst.CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            inst.CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // renders
        if (!::IsIconic(hwnd)) 
        {
            inst.StatsRender(hwnd, io);
            inst.OverlayRender(hwnd, io);
        } else Sleep(100);
        //

        ImGui::Render();
        ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pSwapChain->Present(1, 0);
    }

    if (MainGuiDestroy())
        return 1;

    return 0;
}

inline void MainGui::StatsRender(HWND hwnd, ImGuiIO io) 
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
}

inline void MainGui::OverlayRender(HWND hwnd, ImGuiIO io) 
{
    ImGuiWindowFlags window_flags = 
        ImGuiWindowFlags_NoDecoration | 
        ImGuiWindowFlags_NoDocking | 
        ImGuiWindowFlags_AlwaysAutoResize | 
        ImGuiWindowFlags_NoSavedSettings | 
        ImGuiWindowFlags_NoNav;

    if (!bitmapset)
    {
        //HBITMAP bitmap = LoadBitmapA(NULL, "C:\\cpp\\src\\textures\\overlay.bmp");
        bmp = Gdiplus::Bitmap::FromFile(L"C:\\cpp\\src\\textures\\overlay.png");
        HBITMAP hbitmap = NULL;
        bmp->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hbitmap);
        BITMAP bitmap;
        GetObject(hbitmap, sizeof(bitmap), (LPVOID)&bitmap);

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = bitmap.bmWidth;
        desc.Height = bitmap.bmHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        ID3D11Texture2D *pTexture = nullptr;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = bitmap.bmBits;
        subResource.SysMemPitch = bitmap.bmWidthBytes;
        subResource.SysMemSlicePitch = 0;
        g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &out_srv);
        pTexture->Release();

        bitmapset = true;
    }

    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("Overlay", NULL, window_flags);
    ImGui::Image((void*)out_srv, ImVec2(bmp->GetWidth(), bmp->GetHeight()));
    ImGui::End();
}

bool MainGui::MainGuiDestroy()
{
    destroyed = true;
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    inst.CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    
    Gdiplus::GdiplusShutdown(token);

    return 0;
}

LRESULT WINAPI MainGui::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_HOTKEY:
        if (lParam == VK_TAB | MOD_NOREPEAT)
        {
            if (::IsIconic(hWnd))
                ::OpenIcon(hWnd);
            else
                ::CloseWindow(hWnd);
        }
        return 0;
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

inline bool MainGui::CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

inline void MainGui::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

inline void MainGui::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

inline void MainGui::CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

ID3D11Texture2D* CreateTexture(const char* path) 
{
    return (ID3D11Texture2D*)nullptr;
}