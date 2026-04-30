/*
 * [포인트: 셰이더 컴파일의 2가지 길]
 * 1. CompileFromString: 코드 내부에 텍스트로 존재 (빠른 테스트용)
 * 2. CompileFromFile: 외부 파일(.hlsl)에서 읽기 (실무 표준)
 *  - Separate Files: VS와 PS를 완전히 다른 파일로 관리 (대규모 프로젝트)
 *  - Unified FX: 하나의 파일 안에 VS/PS 통합 관리
 * * ※ 주의: CreateVertexShader/CreatePixelShader를 호출한 뒤에는
 * 컴파일된 중간 데이터(Blob)를 반드시 Release() 해야 메모리 누수가 없음!

  * [실습 주제: 셰이더 컴파일의 4가지 방식 통합 구현]
  * 설명: 1. 스트링 방식 2. 파일 방식 3. 분리 방식 4. 통합 FX 방식
  * 주의: 프로젝트 폴더에 "Shader.hlsl" 파일이 있어야 파일 방식이 작동함.
  */

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdio.h>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

   // --- [전역 객체] ---
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;
ID3D11InputLayout* g_pVertexLayout = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr;

// --- [방식 1을 위한 셰이더 소스 스트링] ---
const char* g_szShaderCode = R"(
struct VS_OUTPUT { float4 Pos : SV_POSITION; float4 Col : COLOR; };
VS_OUTPUT VS(float3 pos : POSITION, float4 col : COLOR) {
    VS_OUTPUT output;
    output.Pos = float4(pos, 1.0f);
    output.Col = col;
    return output;
}
float4 PS(VS_OUTPUT input) : SV_Target { return input.Col; }
)";

// --- [셰이더 컴파일 헬퍼 함수] ---
HRESULT CompileShader(const void* pSrc, bool isFile, LPCSTR szEntry, LPCSTR szTarget, ID3DBlob** ppBlob) {
    ID3DBlob* pErrorBlob = nullptr;
    HRESULT hr;

    if (isFile) {
        // 방식 2, 3, 4: 파일로부터 컴파일
        hr = D3DCompileFromFile((LPCWSTR)pSrc, nullptr, nullptr, szEntry, szTarget, 0, 0, ppBlob, &pErrorBlob);
    }
    else {
        // 방식 1: 메모리 스트링으로부터 컴파일
        hr = D3DCompile(pSrc, strlen((char*)pSrc), nullptr, nullptr, nullptr, szEntry, szTarget, 0, 0, ppBlob, &pErrorBlob);
    }

    if (FAILED(hr) && pErrorBlob) {
        printf("Shader Error: %s\n", (char*)pErrorBlob->GetBufferPointer());
        pErrorBlob->Release();
    }
    return hr;
}

// --- [윈도우 프로시저] ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    // 1. 창 생성
    WNDCLASSEXW wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0, 0, hInst, nullptr, nullptr, nullptr, nullptr, L"DX11", nullptr };
    RegisterClassExW(&wc);
    HWND hWnd = CreateWindowW(L"DX11", L"Shader Compilation Lab", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, hInst, nullptr);

    // 2. DX11 초기화
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1; sd.BufferDesc.Width = 800; sd.BufferDesc.Height = 600;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd; sd.SampleDesc.Count = 1; sd.Windowed = TRUE;
    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, nullptr, &g_pImmediateContext);

    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();

    // ---------------------------------------------------------
    // 3. 셰이더 로드 (여기서 방식을 선택함)
    // ---------------------------------------------------------
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;

    // [테스트할 방식 하나만 주석 해제하셈]

    // 방식 1: 스트링 컴파일
    //CompileShader(g_szShaderCode, false, "VS", "vs_4_0", &vsBlob);
    //CompileShader(g_szShaderCode, false, "PS", "ps_4_0", &psBlob);

    // 방식 2: 별도 파일 (.hlsl) - 파일이 있어야 작동함
    //CompileShader(L"VS.hlsl", true, "main", "vs_4_0", &vsBlob);
    //CompileShader(L"PS.hlsl", true, "main", "ps_4_0", &psBlob);

    // 방식 3: 외부 통합 파일 (.fx) - 파일이 있어야 작동함
    CompileShader(L"Basic.fx", true, "VS", "vs_4_0", &vsBlob);
    CompileShader(L"Basic.fx", true, "PS", "ps_4_0", &psBlob);
    
    g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pPixelShader);

    // 4. 레이아웃 & 버퍼 설정
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    g_pd3dDevice->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_pVertexLayout);
    vsBlob->Release(); psBlob->Release(); // 생성 끝났으면 Blob은 바로 해제 (메모리 관리!)

    float vertices[] = { 0.0f, 0.5f, 0.0f, 1,0,0,1, 0.5f, -0.5f, 0.0f, 0,1,0,1, -0.5f, -0.5f, 0.0f, 0,0,1,1 };
    D3D11_BUFFER_DESC bd = { sizeof(vertices), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
    D3D11_SUBRESOURCE_DATA init = { vertices, 0, 0 };
    g_pd3dDevice->CreateBuffer(&bd, &init, &g_pVertexBuffer);

    // 5. 루프
    ShowWindow(hWnd, nCmdShow);
    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        else {
            float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
            g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, color);
            g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

            D3D11_VIEWPORT vp = { 0, 0, 800, 600, 0, 1 };
            g_pImmediateContext->RSSetViewports(1, &vp);
            g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
            UINT stride = 28, offset = 0;
            g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
            g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
            g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
            g_pImmediateContext->Draw(3, 0);
            g_pSwapChain->Present(0, 0);
        }
    }

    // 6. [중요] 자원 해제 - 빌려온 건 다 갚고 가야 함
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader)  g_pPixelShader->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    return 0;
}