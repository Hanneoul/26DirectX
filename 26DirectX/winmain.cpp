/*
================================================================================
 [Engine Architecture]
 1. WindowContext: Win32 창 생성 및 메시지 루프 관리
 2. GraphicsContext: DX11 디바이스, 스왑체인, 셰이더 컴파일 및 영상 설정 관리
 3. DeltaTime: 고해상도 타이머를 이용한 시간 계산
 4. GameObject & Component: 객체 지향적 기능 확장 구조
 5. GameLoop: 전체 흐름(Input-Update-Render) 제어
================================================================================
*/

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <vector>
#include <chrono>
#include <string>
#include <random>

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 pos; XMFLOAT4 col;
};

struct ConstantBuffer
{
    XMMATRIX matWorld;
};

struct Mesh
{
    ID3D11Buffer* vBuffer;
    ID3D11InputLayout* pInputLayout;
    ID3D11VertexShader* pVS;
    ID3D11PixelShader* pPS;
    UINT vertexCount;
    XMFLOAT4 color;

    Mesh()
        : vBuffer(NULL), pInputLayout(NULL), pVS(NULL), pPS(NULL), vertexCount(0)
    {
        color = { 1, 1, 1, 1 };
    }

    ~Mesh()
    {
        if (vBuffer) vBuffer->Release();
        if (pInputLayout) pInputLayout->Release();
        if (pVS) pVS->Release();
        if (pPS) pPS->Release();
    }
};

class DeltaTime
{
    std::chrono::high_resolution_clock::time_point prevTime;
public:
    DeltaTime()
    {
        prevTime = std::chrono::high_resolution_clock::now();
    }

    float GetDelta()
    {
        std::chrono::steady_clock::time_point currTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(currTime - prevTime).count();
        prevTime = currTime;
        return dt;
    }
};

class WindowContext
{
public:
    HWND hWnd;
    int Width, Height;
    LPCWSTR windowName;

    WindowContext(LPCWSTR winName = L"DX11 Component Engine")
        : windowName(winName), hWnd(nullptr), Width(800), Height(600)
    {
    }

    ~WindowContext()
    {
        UnregisterClass(L"DX11Engine", GetModuleHandle(NULL));
    }

    bool Initialize(HINSTANCE hInst, int w, int h, LRESULT(CALLBACK* wndProc)(HWND, UINT, WPARAM, LPARAM))
    {
        Width = w; Height = h;

        WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = wndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = L"DX11Engine";

        if (!RegisterClassEx(&wc)) return false;

        RECT rc = { 0, 0, w, h };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        hWnd = CreateWindow(L"DX11Engine", windowName, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
            NULL, NULL, hInst, NULL);

        if (!hWnd) return false;

        ShowWindow(hWnd, SW_SHOW);
        return true;
    }
};

class GraphicsContext {
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* ImmediateContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
    ID3D11RenderTargetView* RTV = nullptr;

    bool IsFullscreen = false;
    int VSync = 1;

    bool InitDX(HWND hWnd, int w, int h)
    {
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

    bool CreateRTV(int w, int h)
    {
        if (RTV) RTV->Release();
        ID3D11Texture2D* pBB;
        SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBB);
        Device->CreateRenderTargetView(pBB, NULL, &RTV);
        pBB->Release();
        return true;
    }

    void Resize(int w, int h)
    {
        ImmediateContext->OMSetRenderTargets(0, 0, 0);
        RTV->Release(); RTV = nullptr;
        SwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
        CreateRTV(w, h);
    }

    void SetFullscreen(bool goFull)
    {
        IsFullscreen = goFull;
        SwapChain->SetFullscreenState(goFull, NULL);
    }

    ID3DBlob* CompileShader(const std::string& src, const std::string& entry, const std::string& profile) {
        ID3DBlob* blob = nullptr;
        D3DCompile(src.c_str(), src.length(), NULL, NULL, NULL, entry.c_str(), profile.c_str(), 0, 0, &blob, NULL);
        return blob;
    }

    ~GraphicsContext() {
        if (RTV)
            RTV->Release();
        if (SwapChain)
            SwapChain->Release();
        if (ImmediateContext)
            ImmediateContext->Release();
        if (Device)
            Device->Release();
    }
};

class GameObject;
class Component
{
public:
    GameObject* pOwner = nullptr;
    bool isStarted = false;

    Component() {}
    virtual void Start(GraphicsContext* gfx) = 0;
    virtual void Input() = 0; // 컴포넌트 레벨의 입력 처리
    virtual void Update(float dt) = 0;
    virtual void Render(GraphicsContext* gfx) = 0;
    virtual ~Component() {}
};

class GameObject
{
public:
    XMFLOAT3 pos = { 0, 0, 0 };
    XMFLOAT3 rot = { 0, 0, 0 };
    XMFLOAT3 scale = { 1, 1, 1 };
    std::vector<Component*> components;

    GameObject(float x, float y, float z)
    {
        pos.x = x;
        pos.y = y;
        pos.z = z;
    }
    ~GameObject()
    {
        for (int i = 0; i < (int)components.size(); i++)
            delete components[i];
    }

    void AddComponent(Component* c)
    {
        c->pOwner = this;
        components.push_back(c);
    }

    void Input()
    {
        // 인덱스 기반 루프로 하위 컴포넌트의 Input 호출
        int componentCount = (int)components.size();
        for (int i = 0; i < componentCount; i++)
        {
            if (components[i] != nullptr)
            {
                components[i]->Input();
            }
        }
    }

    void Update(float dt, GraphicsContext* gfx)
    {
        for (int j = 0; j < (int)components.size(); j++)
        {
            if (components[j] != nullptr)
            {
                if (components[j]->isStarted == false)
                {
                    components[j]->Start(gfx);
                    components[j]->isStarted = true;
                }

                components[j]->Update(dt);
            }
        }
    }
    void Render(GraphicsContext* gfx)
    {
        for (int i = 0; i < components.size(); i++)
        {
            if (components[i] != nullptr)
            {
                components[i]->Render(gfx);
            }
        }
    }
};

class MeshRenderer : public Component
{
    Mesh* pMeshData = nullptr;
    ID3D11Buffer* cBuffer = nullptr;

public:
    MeshRenderer(Mesh* mesh) : Component(), pMeshData(mesh)
    {
    }

    ~MeshRenderer()
    {
        if (cBuffer) cBuffer->Release();
        if (pMeshData) delete pMeshData;
    }

    void Start(GraphicsContext* gfx) override
    {
        D3D11_BUFFER_DESC cbd = { 0 };
        cbd.Usage = D3D11_USAGE_DEFAULT;
        cbd.ByteWidth = sizeof(ConstantBuffer);
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        gfx->Device->CreateBuffer(&cbd, nullptr, &cBuffer);
    }

    void Render(GraphicsContext* gfx) override
    {
        if (pMeshData == nullptr || pMeshData->vBuffer == nullptr) return;

        gfx->ImmediateContext->IASetInputLayout(pMeshData->pInputLayout);
        gfx->ImmediateContext->VSSetShader(pMeshData->pVS, nullptr, 0);
        gfx->ImmediateContext->PSSetShader(pMeshData->pPS, nullptr, 0);

        float s = 1.0f / (pOwner->pos.z + 1.0f);
        XMMATRIX world = XMMatrixScaling(s, s, s) * XMMatrixRotationZ(pOwner->rot.z) * XMMatrixTranslation(pOwner->pos.x, pOwner->pos.y, 0.0f);
        ConstantBuffer cb;
        cb.matWorld = XMMatrixTranspose(world);
        gfx->ImmediateContext->UpdateSubresource(cBuffer, 0, nullptr, &cb, 0, 0);

        UINT stride = sizeof(Vertex), offset = 0;
        gfx->ImmediateContext->IASetVertexBuffers(0, 1, &pMeshData->vBuffer, &stride, &offset);
        gfx->ImmediateContext->VSSetConstantBuffers(0, 1, &cBuffer);
        gfx->ImmediateContext->Draw(pMeshData->vertexCount, 0);
    }
    void Input() override {}
    void Update(float dt) override {}
};

class PlayerController : public Component
{
    // 입력 상태를 저장하기 위한 멤버 변수 (내부용)
    XMFLOAT2 moveDir;  // x: 좌우, y: 상하
    float    rotDir;   // 회전 방향
    float    zoomDir;  // 확대/축소 방향

public:
    PlayerController() : Component()
    {
        moveDir = { 0, 0 };
        rotDir = 0.0f;
        zoomDir = 0.0f;
    }

    ~PlayerController()
    {
    }

    void Start(GraphicsContext* gfx) override
    {
    }

    // [Step 1] 입력 감지 및 상태 저장
    void Input() override
    {
        // 매 프레임 입력 상태 초기화
        moveDir = { 0, 0 };
        rotDir = 0.0f;
        zoomDir = 0.0f;

        // 방향키 입력 (이동)
        if (GetAsyncKeyState(VK_UP) & 0x8000)    moveDir.y += 1.0f;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000)  moveDir.y -= 1.0f;
        if (GetAsyncKeyState(VK_LEFT) & 0x8000)  moveDir.x -= 1.0f;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) moveDir.x += 1.0f;

        // AD 키 입력 (회전)
        if (GetAsyncKeyState('A') & 0x8000) rotDir += 1.0f;
        if (GetAsyncKeyState('D') & 0x8000) rotDir -= 1.0f;

        // WS 키 입력 (줌)
        if (GetAsyncKeyState('W') & 0x8000) zoomDir -= 1.0f;
        if (GetAsyncKeyState('S') & 0x8000) zoomDir += 1.0f;
    }

    // [Step 2] 저장된 상태를 바탕으로 데이터 갱신
    void Update(float dt) override
    {
        // 1. 속도 정의 (사이즈 비례 속도 적용 가능)
        float speedFactor = pOwner->scale.x;
        float moveSpeed = 2.0f * speedFactor;
        float rotateSpeed = 3.0f * speedFactor;
        float zoomSpeed = 5.0f * speedFactor;

        // 2. 위치 업데이트
        pOwner->pos.x += moveDir.x * moveSpeed * dt;
        pOwner->pos.y += moveDir.y * moveSpeed * dt;

        // 3. 회전 업데이트
        pOwner->rot.z += rotDir * rotateSpeed * dt;

        // 4. 줌(Z축) 업데이트 및 제한
        pOwner->pos.z += zoomDir * zoomSpeed * dt;

        if (pOwner->pos.z < -0.9f)
        {
            pOwner->pos.z = -0.9f;
        }
    }

    void Render(GraphicsContext* gfx) override
    {
    }
};

class GameLoop
{
public:
    WindowContext win;
    GraphicsContext gfx;
    DeltaTime timer;
    std::vector<GameObject*> world;
    bool isRunning = true;

    ID3D11VertexShader* pDefaultVS = nullptr;
    ID3D11PixelShader* pDefaultPS = nullptr;
    ID3D11InputLayout* pDefaultLayout = nullptr;

    GameLoop() : isRunning(true)
    {
        world.clear();
        printf("[Engine] GameLoop Created.\n");
    }

    ~GameLoop()
    {
        for (int i = 0; i < (int)world.size(); i++)
        {
            if (world[i])
            {
                delete world[i];
                world[i] = nullptr;
            }
        }
        world.clear();

        if (pDefaultLayout) pDefaultLayout->Release();
        if (pDefaultVS) pDefaultVS->Release();
        if (pDefaultPS) pDefaultPS->Release();

        printf("[Engine] GameLoop Destroyed. All resources released.\n");
    }

    void Initialize(HINSTANCE hInst, LRESULT(CALLBACK* wndProc)(HWND, UINT, WPARAM, LPARAM))
    {
        win.Initialize(hInst, 800, 600, wndProc);
        gfx.InitDX(win.hWnd, 800, 600);
    }

    void Input()
    {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            isRunning = false;
        if (GetAsyncKeyState('F') & 0x0001)
            gfx.SetFullscreen(!gfx.IsFullscreen);

        if (GetAsyncKeyState('C') & 0x0001) // 0x0001은 이번 프레임에 눌렸는지 확인(Toggle)
        {
            // 1. 내부 변수 업데이트
            win.Width = 600;
            win.Height = 600;

            // 2. 실제 Win32 윈도우 크기 변경
            // SWP_NOMOVE: 위치는 유지, SWP_NOZORDER: 레이어 순서 유지
            RECT rc = { 0, 0, win.Width, win.Height };
            AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

            SetWindowPos(win.hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

            // 3. DX11 백버퍼 및 RTV 리사이즈 (GraphicsContext에 정의된 함수 호출)
            gfx.Resize(win.Width, win.Height);

            printf("[Engine] Window Resized to 600x600\n");
        }

        // 2. 월드 내 모든 오브젝트에 입력 전파
        int objectCount = (int)world.size();
        for (int i = 0; i < objectCount; i++)
        {
            if (world[i] != nullptr)
            {
                world[i]->Input();
            }
        }
    }

    void Update()
    {
        float dt = timer.GetDelta();
        for (int i = 0; i < (int)world.size(); i++)
        {
            if (world[i] != nullptr)
            {
                world[i]->Update(dt, &gfx);
            }
        }
    }

    void Render()
    {
        float col[] = { 0.1f, 0.2f, 0.3f, 1.0f };
        gfx.ImmediateContext->ClearRenderTargetView(gfx.RTV, col);

        D3D11_VIEWPORT vp = { 0, 0, (float)win.Width, (float)win.Height, 0, 1 };
        gfx.ImmediateContext->RSSetViewports(1, &vp);
        gfx.ImmediateContext->OMSetRenderTargets(1, &gfx.RTV, NULL);

        if (pDefaultLayout)
        {
            gfx.ImmediateContext->IASetInputLayout(pDefaultLayout);
        }
        gfx.ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for (int i = 0; i < (int)world.size(); i++)
        {
            if (world[i] != nullptr)
            {
                world[i]->Render(&gfx);
            }
        }
        gfx.SwapChain->Present(gfx.VSync, 0);
    }

    void Run()
    {
        MSG msg = {};
        while (msg.message != WM_QUIT && isRunning)
        {
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg); DispatchMessage(&msg);
            }
            else
            {
                Input();
                Update();
                Render();
            }
        }
    }
};

LRESULT CALLBACK GlobalWndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE, LPSTR, int nS)
{
    GameLoop gEngine;
    gEngine.Initialize(hI, GlobalWndProc);

    std::string triShader = R"(
        cbuffer cb0 : register(b0) { matrix matWorld; };

        struct VS_IN { float3 pos : POSITION; float4 col : COLOR; };
        struct PS_IN { float4 pos : SV_POSITION; float4 col : COLOR; };
        
        PS_IN VS(VS_IN input) 
        {
            PS_IN output;
            output.pos = mul(float4(input.pos, 1.0f), matWorld);
            output.col = input.col;
            return output;
        }

        float4 PS(PS_IN input) : SV_Target { return input.col; }
    )";
    ID3DBlob* vsBlob = gEngine.gfx.CompileShader(triShader, "VS", "vs_5_0");
    ID3DBlob* psBlob = gEngine.gfx.CompileShader(triShader, "PS", "ps_5_0");


    // ====================================================
    //  황금별 (Player)
    // ====================================================
    Mesh* goldMesh = new Mesh();
    goldMesh->color = { 1.0f, 0.85f, 0.0f, 1.0f }; // 황금색
    goldMesh->vertexCount = 30;

    float outerR = 0.5f; float innerR = 0.2f;
    XMFLOAT3 p[10];
    for (int i = 0; i < 10; ++i)
    {
        float r;

        if (i % 2 == 0)
        {
            r = outerR; // 바깥쪽 반지름(outerR)을 대입
        }
        else
        {
            r = innerR; // 안쪽 반지름(innerR)을 대입
        }
        float angle = XM_PIDIV2 - (i * XM_2PI / 10.0f);
        p[i] = { cosf(angle) * r, sinf(angle) * r, 0.0f };
    }

    //정석대로 그리기
    std::vector<Vertex> vGold;
    for (int i = 0; i < 10; i++)
    {
        vGold.push_back({ {0,0,0}, goldMesh->color });
        vGold.push_back({ p[i], goldMesh->color });
        vGold.push_back({ p[(i + 1) % 10], goldMesh->color });
    }

    // 리소스 생성 (황금별용)
    gEngine.gfx.Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &goldMesh->pVS);
    gEngine.gfx.Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &goldMesh->pPS);

    D3D11_BUFFER_DESC bd = { 0 };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(Vertex) * (UINT)vGold.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA sd = { vGold.data() };
    gEngine.gfx.Device->CreateBuffer(&bd, &sd, &goldMesh->vBuffer);

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    gEngine.gfx.Device->CreateInputLayout(ied, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &goldMesh->pInputLayout);


    // 황금별 객체 등록 (PlayerController 포함)
    GameObject* gStar = new GameObject(0, 0, 0);
    gStar->scale = { 0.5f, 0.5f, 1.0f };

    // 기존 PlayerController 대신 StarController 사용
    gStar->AddComponent(new MeshRenderer(goldMesh));
    gStar->AddComponent(new PlayerController());

    gEngine.world.push_back(gStar);

    // ====================================================
    // 추가되는 랜덤 별 n개 (Background Stars)
    // ====================================================
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disPos(-1.2f, 1.2f);
    std::uniform_real_distribution<float> disCol(0.3f, 0.9f);
    std::uniform_real_distribution<float> disScale(0.05f, 0.4f); // 최대 0.5 (화면 1/4)

    int n = 20; // 추가할 별 개수
    for (int k = 0; k < n; k++)
    {
        Mesh* randMesh = new Mesh();
        randMesh->color = { disCol(gen), disCol(gen), disCol(gen), 1.0f };
        randMesh->vertexCount = 30;

        std::vector<Vertex> vRand;
        for (int i = 0; i < 10; i++) {
            vRand.push_back({ {0,0,0}, randMesh->color });
            vRand.push_back({ p[i], randMesh->color });
            vRand.push_back({ p[(i + 1) % 10], randMesh->color });
        }

        // 리소스 생성 (각 별마다 고유 색상 버퍼 생성)
        gEngine.gfx.Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &randMesh->pVS);
        gEngine.gfx.Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &randMesh->pPS);

        gEngine.gfx.Device->CreateInputLayout(ied, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &randMesh->pInputLayout);

        bd.ByteWidth = sizeof(Vertex) * (UINT)vRand.size();
        sd.pSysMem = vRand.data();
        gEngine.gfx.Device->CreateBuffer(&bd, &sd, &randMesh->vBuffer);

        GameObject* bgStar = new GameObject(disPos(gen), disPos(gen), 0);
        float s = disScale(gen);
        bgStar->scale = { s, s, 1.0f };

        // 1. 렌더러 추가
        bgStar->AddComponent(new MeshRenderer(randMesh));

        // [핵심 추가] 모든 랜덤 별에게도 컨트롤러를 달아줍니다!
        // 이제 이 별들도 키보드 입력에 반응하며, 각자의 s 값에 따라 속도가 결정됩니다.
        bgStar->AddComponent(new PlayerController());

        gEngine.world.push_back(bgStar);
    }

    vsBlob->Release(); psBlob->Release();


    gEngine.Run();

    return 0;
}