#include "graphics.h"

// decl
ID3D11ShaderResourceView* out_srv;
bool bitmapset = false;

Graphics::Graphics(HWND hwnd)
{
    this->hwnd = hwnd;
    pDevice = nullptr;
    pDeviceContext = nullptr;
    pSwapChain = nullptr;
    ResizeWidth = 0;
    ResizeHeight = 0;
    pRenderTargetView = nullptr;
    token = 0;
    bmp = nullptr;
    if (!CreateDeviceD3D())
    {
        CleanupDeviceD3D();
        return;
    }

    Gdiplus::GdiplusStartupInput input;
    Gdiplus::GdiplusStartup(&token, &input, 0);
}

bool Graphics::OverlayDraw() 
{
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
        pDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        pDevice->CreateShaderResourceView(pTexture, &srvDesc, &out_srv);
        pTexture->Release();

        bitmapset = true;
    }

    return 0;
}

bool Graphics::OverlayRender()
{
    const float clear_color_with_alpha[4] = { 1.f, 1.f, 1.f, 0.f };
    pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
    pDeviceContext->ClearRenderTargetView(pRenderTargetView, clear_color_with_alpha);

    pSwapChain->Present(1, 0);

    return 0;
}

void Graphics::Resize(UINT g_ResizeWidth, UINT g_ResizeHeight)
{
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        CleanupRenderTarget();
        pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }
}

inline bool Graphics::CreateDeviceD3D()
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
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, &featureLevel, &pDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, &featureLevel, &pDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

inline void Graphics::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (pSwapChain) { pSwapChain->Release(); pSwapChain = nullptr; }
    if (pDeviceContext) { pDeviceContext->Release(); pDeviceContext = nullptr; }
    if (pDevice) { pDevice->Release(); pDevice = nullptr; }
}

inline void Graphics::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
    pBackBuffer->Release();
}

inline void Graphics::CleanupRenderTarget()
{
    if (pRenderTargetView) { pRenderTargetView->Release(); pRenderTargetView = nullptr; }
}

Graphics::~Graphics()
{
    CleanupDeviceD3D();
    Gdiplus::GdiplusShutdown(token);
}