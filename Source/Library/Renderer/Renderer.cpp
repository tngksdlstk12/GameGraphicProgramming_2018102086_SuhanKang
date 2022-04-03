#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1, 
                  m_immediateContext, m_immediateContext1, m_swapChain, 
                  m_swapChain1, m_renderTargetView].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Renderer definition (remove the comment)
    --------------------------------------------------------------------*/
    Renderer::Renderer() {
        m_driverType = D3D_DRIVER_TYPE_NULL;
        m_featureLevel = D3D_FEATURE_LEVEL_11_0;
        m_d3dDevice = nullptr;
        m_d3dDevice1 = nullptr;
        m_immediateContext = nullptr;
        m_immediateContext1= nullptr;
        m_swapChain = nullptr;
        m_swapChain1 = nullptr;
        m_renderTargetView = nullptr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext, 
                  m_d3dDevice1, m_immediateContext1, m_swapChain1, 
                  m_swapChain, m_renderTargetView].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Initialize definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd) {
        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_9_1,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_11_1
        };
        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        // This flag adds support for surfaces with a color-channel ordering different
        // from the API default. It is required for compatibility with Direct2D.
        UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        // Create the Direct3D 11 API device object and a corresponding context.
        D3D_FEATURE_LEVEL featureLevel;
        HRESULT hr = D3D11CreateDevice(
            nullptr,                    // Specify nullptr to use the default adapter.
            D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
            0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
            deviceFlags,                // Set debug and Direct2D compatibility flags.
            levels,                     // List of feature levels this app can support.
            ARRAYSIZE(levels),          // Size of the list above.
            D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
            m_d3dDevice.GetAddressOf(),                    // Returns the Direct3D device created.
            &featureLevel,            // Returns feature level of device created.
            m_immediateContext.GetAddressOf()                   // Returns the device immediate context.
        );
        if (FAILED(hr))
        {
            return hr;
        }
        /*--------------------------------------------*/
        /*-----------------------------------------Get DXGI FACTORY*/
        Microsoft::WRL::ComPtr<IDXGIDevice> pDXGIDevice;
        hr = m_d3dDevice.As(&pDXGIDevice);
        if (FAILED(hr))
        {
            return hr;
        }
        Microsoft::WRL::ComPtr<IDXGIAdapter> pDXGIAdapter;
        hr = pDXGIDevice->GetAdapter(pDXGIAdapter.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }
        Microsoft::WRL::ComPtr<IDXGIFactory> pIDXGIFactory;
        pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);
        if (FAILED(hr))
        {
            return hr;
        }
        /*----------------------------------------*/
        /*-------------------------------------------Create Swap Chain*/

        DXGI_RATIONAL refreshRate = {
            .Numerator = 60,
            .Denominator = 1
        };

        DXGI_MODE_DESC bufferDesc = {
            .Width = width,
            .Height = height,
            .RefreshRate = refreshRate,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM
        };
        
        DXGI_SAMPLE_DESC sampleDesc = {
            .Count = 1,
            .Quality = 0
        };

        DXGI_SWAP_CHAIN_DESC desc = {
            .BufferDesc = bufferDesc,
            .SampleDesc = sampleDesc,
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 1,
            .OutputWindow = hWnd,
            .Windowed = TRUE
        };

        

        hr = pIDXGIFactory->CreateSwapChain(
            m_d3dDevice.Get(),
            &desc,
            m_swapChain.GetAddressOf()
        );
        if (FAILED(hr))
        {
            return hr;
        }
        /*------------------------------------------*/
        /*-------------------------------------------Create Render Target*/
        Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;
        pBackBuffer.Reset();

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width;
        vp.Height = (FLOAT)height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_immediateContext->RSSetViewports(1, &vp);
        /*-------------------------------------------*/
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Initialize definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::Render() {
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);
        m_swapChain->Present(0, 0);
    }
}