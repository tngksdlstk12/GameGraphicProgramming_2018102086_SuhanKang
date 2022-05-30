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
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice()
        , m_d3dDevice1()
        , m_immediateContext()
        , m_immediateContext1()
        , m_swapChain()
        , m_swapChain1()
        , m_renderTargetView()
        , m_depthStencil()
        , m_depthStencilView()
        , m_cbChangeOnResize()
        , m_pszMainSceneName(nullptr)
        , m_padding{ '\0' }
        , m_camera(XMVectorSet(0.0f, 3.0f, -6.0f, 0.0f))
        , m_projection()
        , m_scenes()
        , m_invalidTexture(std::make_shared<Texture>(L"Content/Common/InvalidTexture.png"))
    {
    }


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
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT>(rc.right - rc.left);
        UINT uHeight = static_cast<UINT>(rc.bottom - rc.top);

        UINT uCreateDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (SUCCEEDED(hr))
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u, .Quality = 0u },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth, .Height = uHeight, .RefreshRate = {.Numerator = 60, .Denominator = 1 }, .Format = DXGI_FORMAT_R8G8B8A8_UNORM },
                .SampleDesc = {.Count = 1, .Quality = 0 },
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = uWidth,
            .Height = uHeight,
            .MipLevels = 1u,
            .ArraySize = 1u,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1u, .Quality = 0u },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };
        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        m_immediateContext->RSSetViewports(1, &vp);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // Create the constant buffers
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(CBChangeOnResize),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
            .CPUAccessFlags = 0
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize the projection matrix
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<FLOAT>(uWidth) / static_cast<FLOAT>(uHeight), 0.01f, 1000.0f);

        CBChangeOnResize cbChangesOnResize =
        {
            .Projection = XMMatrixTranspose(m_projection)
        };
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0);

        bd.ByteWidth = sizeof(CBLights);
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0u;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_camera.Initialize(m_d3dDevice.Get());

        if (!m_scenes.contains(m_pszMainSceneName))
        {
            return E_FAIL;
        }

        hr = m_scenes[m_pszMainSceneName]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_invalidTexture->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add scene to renderer

      Args:     PCWSTR pszSceneName
                  The name of the scene
                const std::shared_ptr<Scene>&
                  The shared pointer to Scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, _In_ const std::shared_ptr<Scene>& scene)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_scenes[pszSceneName] = scene;

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetSceneOrNull

      Summary:  Return scene with the given name or null

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Returns:  std::shared_ptr<Scene>
                  The shared pointer to Scene
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    std::shared_ptr<Scene> Renderer::GetSceneOrNull(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return m_scenes[pszSceneName];
        }

        return nullptr;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  The name of the scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (!m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }

        m_pszMainSceneName = pszSceneName;

        return S_OK;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Handle user mouse input

      Args:     DirectionsInput& directions
                MouseRelativeMovement& mouseRelativeMovement
                FLOAT deltaTime
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        m_scenes[m_pszMainSceneName]->Update(deltaTime);

        m_camera.Update(deltaTime);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render() {

        //clear back buffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);
        //clear the depth buffer
        m_immediateContext->ClearDepthStencilView(
            m_depthStencilView.Get(),
            D3D11_CLEAR_DEPTH,
            1.0f,
            0);

        //create camera cojnstant buffer and update

        XMFLOAT4 cameraPosition = XMFLOAT4();
        XMStoreFloat4(&cameraPosition, m_camera.GetEye());
        CBChangeOnCameraMovement cbChangeOnCamera = {
               .View = XMMatrixTranspose(m_camera.GetView()),
               .CameraPosition = cameraPosition
        };
        m_immediateContext->UpdateSubresource(
            m_camera.GetConstantBuffer().Get(),
            0,
            nullptr,
            &cbChangeOnCamera,
            0,
            0
        );

        
        
        for (auto it_Scene = m_scenes.begin(); it_Scene != m_scenes.end(); it_Scene++) {
            CBLights cbLight = {
            .LightPositions = {},
            .LightColors = {}
            };
            for (int i = 0; i < NUM_LIGHTS; i++)
            {
                cbLight.LightPositions[i] = it_Scene->second->GetPointLight(i)->GetPosition();
                cbLight.LightColors[i] = it_Scene->second->GetPointLight(i)->GetColor();
            }

            m_immediateContext->UpdateSubresource(
                m_cbLights.Get(),
                0,
                nullptr,
                &cbLight,
                0,
                0
            );

            //for each renderables

            for (auto it_renderable = it_Scene->second->GetRenderables().begin(); it_renderable != it_Scene->second->GetRenderables().end(); it_renderable++) {
                //create renderable constant buffer and update
                // 
                //set the vertex buffer, index buffer, input layout
                UINT uStride[2] = { sizeof(SimpleVertex), sizeof(NormalData) };
                UINT uOffset[2] = { 0,0 };
                ComPtr<ID3D11Buffer> vertexNormalBuffers[2] = 
                { it_renderable->second->GetVertexBuffer(), it_renderable->second->GetNormalBuffer() };

                m_immediateContext->IASetVertexBuffers(
                    0u,
                    2u,
                    vertexNormalBuffers->GetAddressOf(),
                    uStride,
                    uOffset
                );
                m_immediateContext->IASetIndexBuffer(it_renderable->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
                
                m_immediateContext->IASetInputLayout(it_renderable->second->GetVertexLayout().Get());

                //update constant buffer
                CBChangesEveryFrame cb = {
                    .World = XMMatrixTranspose(it_renderable->second->GetWorldMatrix()),
                    .OutputColor = it_renderable->second->GetOutputColor(),
                    .HasNormalMap = it_renderable->second->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(it_renderable->second->GetConstantBuffer().Get(), 0, nullptr, &cb, 0, 0);
                //set shaders and constant buffers, shader resources, and samplers
                m_immediateContext->VSSetShader(it_renderable->second->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2u, 1u, it_renderable->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2u, 1u, it_renderable->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
                m_immediateContext->PSSetShader(it_renderable->second->GetPixelShader().Get(), nullptr, 0u);
                if (it_renderable->second->HasTexture()) {
                    for (UINT i = 0; i < it_renderable->second->GetNumMeshes(); i++) {
                        UINT materialIndex = it_renderable->second->GetMesh(i).uMaterialIndex;

                        ComPtr<ID3D11ShaderResourceView> shaderResources[2] =
                        { it_renderable->second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView(),
                        it_renderable->second->GetMaterial(materialIndex)->pNormal->GetTextureResourceView() };
                        m_immediateContext->PSSetShaderResources(0, 2, shaderResources->GetAddressOf());

                        ComPtr<ID3D11SamplerState> samplerStates[2] =
                        { it_renderable->second->GetMaterial(materialIndex)->pDiffuse->GetSamplerState(),
                        it_renderable->second->GetMaterial(materialIndex)->pNormal->GetSamplerState() };
                        m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                        m_immediateContext->DrawIndexed(it_renderable->second->GetMesh(i).uNumIndices, it_renderable->second->GetMesh(i).uBaseIndex, it_renderable->second->GetMesh(i).uBaseVertex);
                    }
                }
                else {
                    m_immediateContext->DrawIndexed(it_renderable->second->GetNumIndices(), 0, 0);
                }
            }
            
            std::vector<std::shared_ptr<Voxel>> voxels = it_Scene->second->GetVoxels();
            for (int i = 0; i < voxels.size(); i++) {
                UINT strides[3] = { sizeof(SimpleVertex), sizeof(NormalData), sizeof(InstanceData) };
                UINT offsets[3] = { 0, 0, 0 };
                ComPtr<ID3D11Buffer> vertexInstanceBuffers[3] = 
                { voxels[i]->GetVertexBuffer(), voxels[i]->GetNormalBuffer(), voxels[i]->GetInstanceBuffer() };
                m_immediateContext->IASetVertexBuffers(
                    0u,
                    3u,
                    vertexInstanceBuffers->GetAddressOf(),
                    strides,
                    offsets
                );
                m_immediateContext->IASetIndexBuffer(voxels[i]->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
                m_immediateContext->IASetInputLayout(voxels[i]->GetVertexLayout().Get());


                //update constant buffer
                CBChangesEveryFrame cb = {
                    .World = XMMatrixTranspose(voxels[i]->GetWorldMatrix()),
                    .OutputColor = voxels[i]->GetOutputColor(),
                    .HasNormalMap = voxels[i]->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(voxels[i]->GetConstantBuffer().Get(), 0, nullptr, &cb, 0, 0);
                //set shaders and constant buffers, shader resources, and samplers
                m_immediateContext->VSSetShader(voxels[i]->GetVertexShader().Get(), nullptr, 0u);
                m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2u, 1u, voxels[i]->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2u, 1u, voxels[i]->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
                m_immediateContext->PSSetShader(voxels[i]->GetPixelShader().Get(), nullptr, 0u);

                if (voxels[i]->HasTexture())
                {
                    ComPtr<ID3D11ShaderResourceView> shaderResources[2] = { voxels[i]->GetMaterial(0)->pDiffuse->GetTextureResourceView(),
                                                    voxels[i]->GetMaterial(0)->pNormal->GetTextureResourceView() };
                    ComPtr<ID3D11SamplerState> samplerStates[2] = { voxels[i]->GetMaterial(0)->pDiffuse->GetSamplerState(),
                                                    voxels[i]->GetMaterial(0)->pNormal->GetSamplerState() };
                    m_immediateContext->PSSetShaderResources(0, 2, shaderResources->GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 2, samplerStates->GetAddressOf());
                    m_immediateContext->DrawIndexedInstanced(voxels[i]->GetNumIndices(), voxels[i]->GetNumInstances(), 0, 0, 0);

                }
                else
                {
                    m_immediateContext->DrawIndexedInstanced(voxels[i]->GetNumIndices(), voxels[i]->GetNumInstances(), 0, 0, 0);
                }

            }


            for (auto it_model = it_Scene->second->GetModels().begin(); it_model != it_Scene->second->GetModels().end(); it_model++) {
                //create renderable constant buffer and update
                // 
                //set the vertex buffer, index buffer, input layout


                UINT strides[3] = { static_cast<UINT>(sizeof(SimpleVertex)),static_cast<UINT>(sizeof(NormalData)), static_cast<UINT>(sizeof(AnimationData)) };
                UINT offsets[3] = { 0, 0, 0};
                ComPtr<ID3D11Buffer> vertexAnimationBuffers[3] = { it_model->second->GetVertexBuffer(), it_model->second->GetNormalBuffer(), it_model->second->GetAnimationBuffer() };

                m_immediateContext->IASetVertexBuffers(
                    0u,
                    3u,
                    vertexAnimationBuffers->GetAddressOf(),
                    strides,
                    offsets
                );
                m_immediateContext->IASetIndexBuffer(it_model->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
                m_immediateContext->IASetInputLayout(it_model->second->GetVertexLayout().Get());




                //update constant buffer
                CBChangesEveryFrame cb = {
                    .World = XMMatrixTranspose(it_model->second->GetWorldMatrix()),
                    .OutputColor = it_model->second->GetOutputColor(),
                    .HasNormalMap = it_model->second->HasNormalMap()
                };
                m_immediateContext->UpdateSubresource(it_model->second->GetConstantBuffer().Get(), 0, nullptr, &cb, 0, 0);


                CBSkinning cbSk = {
                    .BoneTransforms = {}
                };
                for (UINT i = 0; i < it_model->second->GetBoneTransforms().size(); i++) {
                    cbSk.BoneTransforms[i] = XMMatrixTranspose(it_model->second->GetBoneTransforms()[i]);
                }
                m_immediateContext->UpdateSubresource(
                    it_model->second->GetSkinningConstantBuffer().Get(),
                    0,
                    nullptr,
                    &cbSk,
                    0,
                    0
                );

                m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2u, 1u, it_model->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(4u, 1u, it_model->second->GetSkinningConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetShader(it_model->second->GetVertexShader().Get(), nullptr, 0);



                m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2u, 1u, it_model->second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
                m_immediateContext->PSSetShader(it_model->second->GetPixelShader().Get(), nullptr, 0u);

                if (it_model->second->HasTexture()) {
                    for (UINT i = 0; i < it_model->second->GetNumMeshes(); i++) {
                        UINT materialIndex = it_model->second->GetMesh(i).uMaterialIndex;
                        ComPtr<ID3D11ShaderResourceView> shaderResources[2] = 
                        { it_model->second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView() ,
                        it_model->second->GetMaterial(materialIndex)->pNormal->GetTextureResourceView() };
                        ComPtr<ID3D11SamplerState> samplerStates[2] = 
                        { it_model->second->GetMaterial(materialIndex)->pDiffuse->GetSamplerState() ,
                        it_model->second->GetMaterial(materialIndex)->pNormal->GetSamplerState() };
                        m_immediateContext->PSSetShaderResources(0, 1, it_model->second->GetMaterial(materialIndex)->pDiffuse->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(0, 1, it_model->second->GetMaterial(materialIndex)->pDiffuse->GetSamplerState().GetAddressOf());
                        m_immediateContext->DrawIndexed(it_model->second->GetMesh(i).uNumIndices, it_model->second->GetMesh(i).uBaseIndex, it_model->second->GetMesh(i).uBaseVertex);
                    }
                }
                else {
                    m_immediateContext->DrawIndexed(it_model->second->GetNumIndices(), 0, 0);
                }
            }
            m_swapChain->Present(0, 0);
        }

    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}