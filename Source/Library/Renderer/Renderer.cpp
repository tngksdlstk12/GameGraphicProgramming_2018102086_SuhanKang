#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_vertexShader,
                  m_pixelShader, m_vertexLayout, m_vertexBuffer].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Renderer definition (remove the comment)
    --------------------------------------------------------------------*/
    Renderer::Renderer() : m_driverType(D3D_DRIVER_TYPE_NULL),
        m_featureLevel(D3D_FEATURE_LEVEL_11_0),
        m_d3dDevice(nullptr),
        m_d3dDevice1(nullptr),
        m_immediateContext(nullptr),
        m_immediateContext1(nullptr),
        m_swapChain(nullptr),
        m_swapChain1(nullptr),
        m_renderTargetView(nullptr),
        m_depthStencil(nullptr),
        m_depthStencilView(nullptr),
        m_view(),
        m_projection(),
        m_renderables(std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>()),
        m_vertexShaders(std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>()),
        m_pixelShaders(std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>())
    {}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView, m_vertexShader,
                  m_vertexLayout, m_pixelShader, m_vertexBuffer].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Initialize definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd) {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - static_cast<UINT>(rc.left);
        UINT height = rc.bottom - static_cast<UINT>(rc.top);

        UINT createDeviceFlags = 0;

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1>           dxgiFactory(nullptr);
        {
            ComPtr<IDXGIDevice>           dxgiDevice(nullptr);
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter>           adapter(nullptr);

                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), (&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
            return hr;
        /*----------------------------------------*/
        /*-------------------------------------------Create Swap Chain*/
        
        // Create swap chain
        ComPtr<IDXGIFactory2>           dxgiFactory2(nullptr);
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SAMPLE_DESC sampleDesk = {
                .Count = 1,
                .Quality = 0
            };
            DXGI_SWAP_CHAIN_DESC1 sd = {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = sampleDesk,
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1
            };


            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
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
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd = {
                .BufferDesc = bufferDesc,
                .SampleDesc = sampleDesc,
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };
            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        hr = dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);


        if (FAILED(hr))
            return hr;

        // Create a render target view
        ComPtr<ID3D11Texture2D>           pBackBuffer(nullptr);

        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport
        D3D11_VIEWPORT vp = {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)width,
            .Height = (FLOAT)height,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_immediateContext->RSSetViewports(1, &vp);


        //create depth-stencil resource
        D3D11_TEXTURE2D_DESC descDepth = {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {
                .Count = 1,
                .Quality = 0,},
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0,
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
            return hr;

        //create depth stencil view

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Flags = 0,
            .Texture2D = {
                .MipSlice = 0,},
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(),
            &descDSV, m_depthStencilView.GetAddressOf());

        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(
            1,
            m_renderTargetView.GetAddressOf(),
            m_depthStencilView.Get());

        //create view and projection matrices
        XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
        XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        m_view = XMMatrixLookAtLH(Eye, At, Up);

        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);

        //iterator initialization
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::const_iterator it_renderable;
        for (it_renderable = m_renderables.begin(); it_renderable != m_renderables.end(); it_renderable++) {
            it_renderable->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        }

        std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>::const_iterator it_vertexshader;
        for (it_vertexshader = m_vertexShaders.begin(); it_vertexshader != m_vertexShaders.end(); it_vertexshader++) {
            it_vertexshader->second->Initialize(m_d3dDevice.Get());
        }

        std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>::const_iterator it_pixelshader;
        for (it_pixelshader = m_pixelShaders.begin(); it_pixelshader != m_pixelShaders.end(); it_pixelshader++) {
            it_pixelshader->second->Initialize(m_d3dDevice.Get());
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable

      Summary:  Add a renderable object and initialize the object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Unique pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddRenderable definition (remove the comment)
    --------------------------------------------------------------------*/

    HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable) {
        if (m_renderables.count(pszRenderableName) != 0)
            return E_FAIL;
        m_renderables.insert(std::make_pair(pszRenderableName, renderable));
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader

      Summary:  Add the vertex shader into the renderer and initialize it

      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add

      Modifies: [m_vertexShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddVertexShader definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader) {
        std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>::const_iterator it_vertexshader;
        for (it_vertexshader = m_vertexShaders.begin(); it_vertexshader != m_vertexShaders.end(); it_vertexshader++) {
            if (pszVertexShaderName == it_vertexshader->first) {
                return E_FAIL;
            }
        }
        m_vertexShaders.insert({ pszVertexShaderName,vertexShader });
        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader

      Summary:  Add the pixel shader into the renderer and initialize it

      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add

      Modifies: [m_pixelShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddPixelShader definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader) {
        std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>::const_iterator it_pixelshader;
        for (it_pixelshader = m_pixelShaders.begin(); it_pixelshader != m_pixelShaders.end(); it_pixelshader++) {
            if (pszPixelShaderName == it_pixelshader->first) {
                return E_FAIL;
            }
        }
        m_pixelShaders.insert({ pszPixelShaderName,pixelShader });
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Update definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::Update(_In_ FLOAT deltaTime) {
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::const_iterator it_renderable;
        for (it_renderable = m_renderables.begin(); it_renderable != m_renderables.end(); it_renderable++) {
            it_renderable->second->Update(deltaTime);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Render definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::Render() {
        //clear back buffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);
        //clear the depth buffer
        m_immediateContext->ClearDepthStencilView(
            m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH,
            1.0f,
            0);
        //for each renderables
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::const_iterator it_renderable;
        for (it_renderable = m_renderables.begin(); it_renderable != m_renderables.end(); it_renderable++) {
            //set the vertex buffer, index buffer, input layout
            UINT uStride = sizeof(SimpleVertex);
            UINT uOffset = 0;
            m_immediateContext->IASetVertexBuffers(
                0u,
                1u,
                it_renderable->second->GetVertexBuffer().GetAddressOf(),
                &uStride,
                &uOffset
            );
            m_immediateContext->IASetIndexBuffer(it_renderable->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            m_immediateContext->IASetInputLayout(it_renderable->second->GetVertexLayout().Get());

            //update constant buffer
            ConstantBuffer cb = {
                .World = XMMatrixTranspose(it_renderable->second->GetWorldMatrix()),
                .View = XMMatrixTranspose(m_view),
                .Projection = XMMatrixTranspose(m_projection),
            };
            m_immediateContext->UpdateSubresource(
                it_renderable->second->GetConstantBuffer().Get(),
                0u,
                nullptr,
                &cb,
                0u,
                0u
            );
            //render the triangles
            m_immediateContext->VSSetShader(
                it_renderable->second->GetVertexShader().Get(),
                nullptr,
                0u
            );
            m_immediateContext->PSSetShader(
                it_renderable->second->GetPixelShader().Get(),
                nullptr,
                0u
            );
            m_immediateContext->VSSetConstantBuffers(
                0u,
                1u,
                it_renderable->second->GetConstantBuffer().GetAddressOf()
            );

            m_immediateContext->DrawIndexed(it_renderable->second->GetNumIndices(), 0, 0);
        }


        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable

      Summary:  Sets the vertex shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetVertexShaderOfRenderable definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::const_iterator it_renderable;
        it_renderable = m_renderables.find(pszRenderableName);
        if (it_renderable == m_renderables.end())
            return E_FAIL;

        std::unordered_map<PCWSTR, std::shared_ptr<VertexShader>>::const_iterator it_vertexshader;
        it_vertexshader = m_vertexShaders.find(pszVertexShaderName);
        if (it_vertexshader == m_vertexShaders.end())
            return E_FAIL;
        it_renderable->second->SetVertexShader(
            it_vertexshader->second
        );
        return S_OK;
    }
    
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable

      Summary:  Sets the pixel shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetPixelShaderOfRenderable definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName) 
    {
        std::unordered_map<PCWSTR, std::shared_ptr<Renderable>>::const_iterator it_renderable;
        it_renderable = m_renderables.find(pszRenderableName);
        if (it_renderable == m_renderables.end())
            return E_FAIL;
        std::unordered_map<PCWSTR, std::shared_ptr<PixelShader>>::const_iterator it_pixelshader;
        it_pixelshader = m_pixelShaders.find(pszPixelShaderName);
        if (it_pixelshader == m_pixelShaders.end())
            return E_FAIL;
        it_renderable->second->SetPixelShader(
            it_pixelshader->second
        );
        return S_OK;
    }
    

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::GetDriverType definition (remove the comment)
    --------------------------------------------------------------------*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const {
        return m_driverType;
    }
}