#include "Shader/VertexShader.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   VertexShader::VertexShader

      Summary:  Constructor

      Args:     PCWSTR pszFileName
                  Name of the file that contains the shader code
                PCSTR pszEntryPoint
                  Name of the shader entry point functino where shader
                  execution begins
                PCSTR pszShaderModel
                  Specifies the shader target or set of shader features
                  to compile against

      Modifies: [m_vertexShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    VertexShader::VertexShader(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR pszShaderModel):
        Shader(pszFileName,pszEntryPoint, pszShaderModel), m_vertexShader(nullptr), m_vertexLayout(nullptr) {}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   VertexShader::Initialize

      Summary:  Initializes the vertex shader and the input layout

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the vertex shader

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT VertexShader::Initialize(_In_ ID3D11Device* pDevice) {
        //compile create vertex shader
        ComPtr<ID3DBlob> pVSBlob;
        HRESULT hr;
        hr = compile(pVSBlob.GetAddressOf());
        if (FAILED(hr)) {
            return hr;
        }

        hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
        if (FAILED(hr)) {
            return hr;
        }
        //input layout object �����
        D3D11_INPUT_ELEMENT_DESC aLayouts[] = {
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot= 0u,
                .AlignedByteOffset = 0u,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0u},
            {
                .SemanticName = "TEXCOORD",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32_FLOAT,
                .InputSlot = 0u,
                .AlignedByteOffset = 12u,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0u
            },
            {
                .SemanticName = "NORMAL",
                .SemanticIndex = 0u,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 0u,
                .AlignedByteOffset = 20u,
                .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0u
            }
        };
        UINT uNumElements = ARRAYSIZE(aLayouts);

        hr = pDevice->CreateInputLayout(
            aLayouts,
            uNumElements,
            pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(),
            m_vertexLayout.GetAddressOf()
        );
        if (FAILED(hr)) {
            return hr;
        }

        //m_immediateContext->IASetInputLayout(m_vertexLayout.Get());

    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   VertexShader::GetVertexShader

      Summary:  Returns the vertex shader

      Returns:  ComPtr<ID3D11VertexShader>&
                  Vertex shader. Could be a nullptr
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11VertexShader>& VertexShader::GetVertexShader() {
        return m_vertexShader;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   VertexShader::GetVertexLayout

      Summary:  Returns the vertex input layout

      Returns:  ComPtr<ID3D11InputLayout>&
                  Vertex input layout
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11InputLayout>& VertexShader::GetVertexLayout() {
        return m_vertexLayout;
    }
}
