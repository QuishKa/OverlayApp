#include "windows.h"
#include "d3d11.h"
#include "gdiplus.h"
#include <wrl.h>

class Graphics
{
public:
    Graphics(const HWND& hwnd);
    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;
    ~Graphics();
    bool OverlayDraw();
    bool OverlayRender();
    void Resize(UINT* g_ResizeWidth, UINT* g_ResizeHeight);
private:
    HWND                                            hwnd;
    Microsoft::WRL::ComPtr<ID3D11Device>            pDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>     pDeviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain>          pSwapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  pRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResource;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   pRasterizerState;
    ULONG_PTR                                       token;
    BITMAP                                          bitmap;
    BITMAP                                          screenBitmap;
    Gdiplus::Bitmap*                                bmp;
    inline bool CreateDeviceD3D();
    inline bool CreateRenderTarget();
    inline void CleanupRenderTarget();
};