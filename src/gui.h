#pragma once

//#include "imgui.h"
#include "graphics.h"
#include <windows.h>
#include <memory>

class OverLayGui // Singleton
{
public:
    OverLayGui(const OverLayGui&) = delete; // no copy
    static bool OverlayInit();
    static bool OverlayStart();
    static void OverlayDestroy() {};

private:
    OverLayGui() {};
    ~OverLayGui();
    static OverLayGui                       inst;
    static HINSTANCE                        hInstance;
    static WNDCLASSEXW                      wc;
    static HWND                             hwnd;
    static bool                             destroyed;
    static std::unique_ptr<Graphics>        render;
    static UINT                             ResizeWidth, ResizeHeight;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};