#pragma once
#include "Framework.hpp"

class GraphicsContext {
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* ImmediateContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
    ID3D11RenderTargetView* RTV = nullptr;
    bool IsFullscreen = false;
    int VSync = 1;

    ~GraphicsContext() {
        if (RTV) RTV->Release();
        if (SwapChain) SwapChain->Release();
        if (ImmediateContext) ImmediateContext->Release();
        if (Device) Device->Release();
    }

    bool InitDX(HWND hWnd, int w, int h) {
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = w; sd.BufferDesc.Height = h;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd; sd.SampleDesc.Count = 1; sd.Windowed = TRUE;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
            D3D11_SDK_VERSION, &sd, &SwapChain, &Device, NULL, &ImmediateContext);

        return SUCCEEDED(hr) && CreateRTV(w, h);
    }

    bool CreateRTV(int w, int h) {
        if (RTV) RTV->Release();
        ID3D11Texture2D* pBB;
        SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBB);
        Device->CreateRenderTargetView(pBB, NULL, &RTV);
        pBB->Release();
        return true;
    }

    void Resize(int w, int h) {
        ImmediateContext->OMSetRenderTargets(0, 0, 0);
        if (RTV) { RTV->Release(); RTV = nullptr; }
        SwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
        CreateRTV(w, h);
    }

    void SetFullscreen(bool goFull) {
        IsFullscreen = goFull;
        SwapChain->SetFullscreenState(goFull, NULL);
    }

    ShaderSet CompileAndCreate(const void* source, size_t length, bool isFile, D3D11_INPUT_ELEMENT_DESC* ied, UINT iedCount)
    {
        ShaderSet res;
        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;
        ID3DBlob* errBlob = nullptr;
        HRESULT hr;

        if (isFile)
        {
            // VS 컴파일
            hr = D3DCompileFromFile((LPCWSTR)source, nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vsBlob, &errBlob);
            if (FAILED(hr))
            {
                if (errBlob)
                {
                    printf("[Shader Error] VS Compile Error (File):\n%s\n", (char*)errBlob->GetBufferPointer());
                    errBlob->Release();
                }
                return res;
            }

            // PS 컴파일
            hr = D3DCompileFromFile((LPCWSTR)source, nullptr, nullptr, "PS", "ps_5_0", 0, 0, &psBlob, &errBlob);
            if (FAILED(hr))
            {
                if (errBlob)
                {
                    printf("[Shader Error] PS Compile Error (File):\n%s\n", (char*)errBlob->GetBufferPointer());
                    errBlob->Release();
                }
                if (vsBlob) vsBlob->Release();
                return res;
            }
        }
        else
        {
            // 메모리 소스 VS 컴파일
            hr = D3DCompile(source, length, nullptr, nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vsBlob, &errBlob);
            if (FAILED(hr))
            {
                if (errBlob)
                {
                    printf("[Shader Error] VS Compile Error:\n%s\n", (char*)errBlob->GetBufferPointer());
                    errBlob->Release();
                }
                return res;
            }

            // 메모리 소스 PS 컴파일
            hr = D3DCompile(source, length, nullptr, nullptr, nullptr, "PS", "ps_5_0", 0, 0, &psBlob, &errBlob);
            if (FAILED(hr))
            {
                if (errBlob)
                {
                    printf("[Shader Error] PS Compile Error:\n%s\n", (char*)errBlob->GetBufferPointer());
                    errBlob->Release();
                }
                if (vsBlob) vsBlob->Release();
                return res;
            }
        }

        // 리소스 생성
        Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &res.vs);
        Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &res.ps);

        if (vsBlob && ied)
        {
            Device->CreateInputLayout(ied, iedCount, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &res.layout);
        }

        if (vsBlob) vsBlob->Release();
        if (psBlob) psBlob->Release();

        return res;
    }
};