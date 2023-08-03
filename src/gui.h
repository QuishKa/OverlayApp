#pragma once

#include "imgui.h"
#include <d3d11.h>

class MainGui // Singleton
{
public:
    MainGui(const MainGui&) = delete; // no copy
    static bool MainGuiInit();
    static bool MainGuiDestroy();

private:
    MainGui() {};
    static MainGui                  inst;
    static ID3D11Device*            g_pd3dDevice;
    static ID3D11DeviceContext*     g_pd3dDeviceContext;
    static IDXGISwapChain*          g_pSwapChain;
    static UINT                     g_ResizeWidth, g_ResizeHeight;
    static ID3D11RenderTargetView*  g_mainRenderTargetView;
    static WNDCLASSEXW              wc;
    static HWND                     hwnd;
    static bool                     destroyed;

    inline bool CreateDeviceD3D(HWND hWnd);
    inline void CleanupDeviceD3D();
    inline void CreateRenderTarget();
    inline void CleanupRenderTarget();
    inline void OverlayRender(HWND hwnd, ImGuiIO io);
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};