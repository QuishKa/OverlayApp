#include "graphics.h"
#include <d2d1.h>
#include <winuser.h>

namespace wrl = Microsoft::WRL;

Graphics::Graphics(const HWND& hwnd)
{
    this->hwnd = hwnd;
    pDevice = nullptr;
    pDeviceContext = nullptr;
    pSwapChain = nullptr;
    pRenderTargetView = nullptr;
    pShaderResource = nullptr;
    pRasterizerState = nullptr;
    bmp = nullptr;
    token = 0;
    if (CreateDeviceD3D())
    {
        return;
    }

    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&token, &input, 0);

    bmp = Gdiplus::Bitmap::FromFile(L"C:\\cpp\\src\\textures\\overlay.png");
    HBITMAP hbitmap = NULL;
    bmp->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hbitmap);
    GetObject(hbitmap, sizeof(bitmap), (LPVOID)&bitmap);
}

bool Graphics::OverlayDraw() 
{
    HRESULT hr = 0;

    D2D1_BITMAP_PROPERTIES bitprops;
    ZeroMemory(&bitprops, sizeof(bitprops));
    bitprops.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED );
    bitprops.dpiX = 96;
    bitprops.dpiY = 96;
    
// get the device context of the screen
  HDC hScreenDC = GetDC(NULL);  
  // and a device context to put it in
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);
  
  // hBitmap is a HBITMAP that i declared globally to use within WM_PAINT
  // maybe worth checking these are positive values
  HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
  // get a new bitmap
  HBITMAP hOldBitmap = (HBITMAP) SelectObject(hMemoryDC, hBitmap);
  BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
  hBitmap = (HBITMAP) SelectObject(hMemoryDC, hOldBitmap);
  // clean up
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL,hScreenDC);
   
    Gdiplus::Bitmap screenBitmapTmp(hBitmap, NULL);
    HBITMAP hTemp = NULL;
    screenBitmapTmp.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hTemp);
    GetObject(hTemp, sizeof(screenBitmap), (LPVOID)&screenBitmap);

    ID2D1Bitmap *pScreen = nullptr;
    D2D1_SIZE_U screenSize = D2D1::SizeU(
            screenBitmap.bmWidth,
            screenBitmap.bmHeight
    );

    //ID3D11Texture2D* tex = nullptr;
    //hr = pDevice->CreateTexture2D( &desc, &initData, &tex );

    ID2D1Factory *pFactory = nullptr;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
    wrl::ComPtr<IDXGISurface1> pBackBuffer;
    //hr = tex->QueryInterface(&pSurface); 
    hr = pSwapChain->GetBuffer(0, __uuidof(IDXGISurface1), &pBackBuffer);
    //pSwapChain->GetBuffer()
    D2D1_RENDER_TARGET_PROPERTIES props =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_IGNORE ),
            96, 96
        );
    ID2D1RenderTarget *p2DRenderTarget = nullptr;
    hr = pFactory->CreateDxgiSurfaceRenderTarget(
        pBackBuffer.Get(),
        props,
        &p2DRenderTarget
    );
    ID2D1Bitmap *pD2bitmap = nullptr;
    ID2D1Bitmap *pD2screenBitmap = nullptr;
    D2D1_SIZE_U size = D2D1::SizeU(
            bitmap.bmWidth,
            bitmap.bmHeight
    );
    hr = p2DRenderTarget->CreateBitmap(
        screenSize,
        bitprops,
        &pD2screenBitmap
    );
    hr = p2DRenderTarget->CreateBitmap(
        size,
        bitprops,
        &pD2bitmap
    );
    D2D1_RECT_U rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = bitmap.bmWidth;
    rect.bottom = bitmap.bmHeight;
    hr = pD2bitmap->CopyFromMemory(&rect, bitmap.bmBits, bitmap.bmWidthBytes);
    rect.left = 0;
    rect.top = 0;
    rect.right = screenBitmap.bmWidth;
    rect.bottom = screenBitmap.bmHeight;
    hr = pD2screenBitmap->CopyFromMemory(&rect, screenBitmap.bmBits, screenBitmap.bmWidthBytes);
    D2D1_SIZE_F scale;
    scale.height = 0.f;
    scale.width = -1.f;
    p2DRenderTarget->BeginDraw();
    D2D1_MATRIX_3X2_F first = D2D1::Matrix3x2F::Rotation(180, D2D1::Point2F(screenBitmap.bmWidth / 2, screenBitmap.bmHeight / 2));
    D2D1_MATRIX_3X2_F second = D2D1::Matrix3x2F(-1, 0, 0, 1, 0, 0);
    D2D1_MATRIX_3X2_F third = D2D1::Matrix3x2F::Translation(screenBitmap.bmWidth, 0);
    p2DRenderTarget->SetTransform(first * second * third);
    p2DRenderTarget->DrawBitmap(
        pD2screenBitmap,
        D2D1::RectF(
                0,
                0,
                screenBitmap.bmWidth,
                screenBitmap.bmHeight),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
    );
    p2DRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    p2DRenderTarget->DrawBitmap(
        pD2bitmap,
        D2D1::RectF(
                0,
                0,
                bitmap.bmWidth,
                bitmap.bmHeight),
        1.0f,
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
    );
    hr = p2DRenderTarget->EndDraw();
    pFactory->Release();
    p2DRenderTarget->Release();
    pD2bitmap->Release();
    pD2screenBitmap->Release();

    return 0;
}

bool Graphics::OverlayRender()
{
    //const float clear_color_with_alpha[4] = { 1.f, 1.f, 1.f, 0.f };
    //pDeviceContext->ClearRenderTargetView(pRenderTargetView.Get(), clear_color_with_alpha);

    pSwapChain->Present(1, 0);

    return 0;
}

void Graphics::Resize(UINT* g_ResizeWidth, UINT* g_ResizeHeight)
{
    if (*g_ResizeWidth != 0 && *g_ResizeHeight != 0)
    {
        CleanupRenderTarget();
        pSwapChain->ResizeBuffers(0, *g_ResizeWidth, *g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        *g_ResizeWidth = *g_ResizeHeight = 0;
        CreateRenderTarget();
    }
}

inline bool Graphics::CreateDeviceD3D()
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;
    sd.BufferDesc.RefreshRate.Denominator = 0;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.Flags = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;//(HWND)696969;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    //D3D_FEATURE_LEVEL featureLevel;
    //const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, nullptr, &pDeviceContext);
    if (hr == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT , nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, nullptr, &pDeviceContext);
    if (hr != S_OK)
        return 1;

    if (CreateRenderTarget())
        return 1;

    return 0;
}

inline bool Graphics::CreateRenderTarget()
{
    HRESULT hr;
    wrl::ComPtr<ID3D11Texture2D> pBackBuffer;
    hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
    if (hr == S_OK)
        hr = pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pRenderTargetView);
    else return 1;

    return 0;
}

inline void Graphics::CleanupRenderTarget()
{
    if (pRenderTargetView.Get() != nullptr) { pRenderTargetView->Release(); pRenderTargetView = nullptr; }
}

Graphics::~Graphics()
{
    if (token != 0)
        Gdiplus::GdiplusShutdown(token);
}