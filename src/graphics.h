#include "d3d11.h"
#include "gdiplus.h"

#ifdef _DEBUG
#define DX11_ENABLE_DEBUG_LAYER
#endif

#ifdef DX11_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

class Graphics
{
public:
    Graphics(HWND hwnd);
    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;
    ~Graphics();
    bool OverlayDraw();
    bool OverlayRender();
    void Resize(UINT g_ResizeWidth, UINT g_ResizeHeight);
private:
    HWND                             hwnd;
    ID3D11Device*                    pDevice;
    ID3D11DeviceContext*             pDeviceContext;
    IDXGISwapChain*                  pSwapChain;
    UINT                             ResizeWidth, ResizeHeight;
    ID3D11RenderTargetView*          pRenderTargetView;
    ULONG_PTR                        token;
    Gdiplus::Bitmap*                 bmp;
    inline bool CreateDeviceD3D();
    inline void CleanupDeviceD3D();
    inline void CreateRenderTarget();
    inline void CleanupRenderTarget();
};