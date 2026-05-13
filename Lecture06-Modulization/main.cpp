/*
 * [강의 : 클래스 다이어그램 - "코드의 계급도와 관계도"]
 *
 * 1. 클래스 다이어그램 (Class Diagram)이란?
 *    - 클래스 내부의 데이터(속성)와 기능(메서드), 그리고 클래스 간의 관계를 표현한 도표.
 *    - 코드를 한 줄도 안 보고 엔진의 전체 구조를 파악할 수 있게 해줌.
 *
 * 2. 관계의 종류 (Relationship):
 *    (1) 상속 (Inheritance): "A는 B의 일종이다" (IS-A)
 *        - 예: ColorMaterial은 Material이다.
 *    (2) 포함/합성 (Composition/Aggregation): "A는 B를 가지고 있다" (HAS-A)
 *        - 예: GameObject는 Component를 가지고 있다.
 *    (3) 의존 (Dependency): "A가 B를 잠시 빌려 쓴다" (USES-A)
 *        - 예: MeshRenderer는 Render()할 때 GraphicsContext를 참조한다.
 *
 * --------------------------------------------------------------------------------------
 * [Engine Core Class Diagram]
 *
 *      +-------------------+
 *      |    GameLoop       | ----------------+ (Owner: 월드 관리)
 *      +-------------------+                 |
 *               | (Has 1:N)                  |
 *               v                            | (Uses)
 *      +-------------------+                 |
 *      |    GameObject     |                 |
 *      +-------------------+                 |
 *      | - pos, rot, scale |                 v
 *      | - components[]    |        +-------------------+
 *      +---------+---------+        |  GraphicsContext  |
 *                | (Has 1:N)        +-------------------+
 *                v                  | - Device          |
 *      +-------------------+        | - DeviceContext   |
 *      |    Component      | <----- | - SwapChain       |
 *      +-------------------+        +-------------------+
 *      | # pOwner (BackRef)|                 ^
 *      +---------+---------+                 |
 *                |                           |
 *       +--------+--------+                  |
 *       | (Inheritance)   |                  |
 *       v                 v                  | (Uses for Binding)
 * +------------+  +------------------+       |
 * | PlayerCtrl |  |   MeshRenderer   |-------+
 * +------------+  +------------------+
 *                 | - pMesh (Has)    |
 *                 | - pMaterial (Has)|
 *                 +--------+---------+
 *                          |
 *                +---------+---------+
 *                |                   |
 *                v                   v
 *      +-------------------+  +-------------------+
 *      |      Mesh         |  |     Material      | (Abstract)
 *      +-------------------+  +-------------------+
 *      | - vBuffer         |  | - ShaderSet       |
 *      | - vertexCount     |  +---------+---------+
 *      +-------------------+            |
 *                                       v (Inheritance)
 *                             +-------------------+
 *                             |   ColorMaterial   |
 *                             +-------------------+
 *                             | - color           |
 *                             | - pColorBuffer    |
 *                             +-------------------+
 * --------------------------------------------------------------------------------------
 */


/*
 * [강의 : 헤더 분할과 전방 선언]
 *
 * 1. 헤더 분할의 철학
 *    - "수정은 좁게, 영향은 적게": 특정 모듈(예: Mesh)을 고쳐도
 *      상관없는 모듈(예: WindowContext)은 다시 컴파일될 필요가 없어야 함.
 *
 * 2. 분할의 기술적 단위 (Logical Units)
 *    - 클래스 다이어그램에서 정의한 '박스' 하나당 '헤더(.h) + 소스(.cpp)' 세트 하나.
 *    - 인터페이스(Interface): 헤더에는 "나 이런 기능 있어"라고 명함만 돌림.
 *    - 구현(Implementation): 소스에는 "실제로는 이렇게 동작해"라고 비밀스럽게 작성함.
 *
 * 3. 전방 선언 (Forward Declaration) - 의존성 지옥 탈출하기
 *    - 문제: A.h가 B.h를 포함하고, B.h가 A.h를 포함하면 컴파일러가 무한 루프에 빠짐.
 *    - 해결: "자세한 건 모르겠고, 아무튼 'class B'라는 게 있어"라고 이름만 알려주는 기법.
 *    - 원칙: 헤더(.h)에서는 가급적 '전방 선언'만 쓰고, 실제 포함(#include)은 소스(.cpp)에서 함.
 *
 * --------------------------------------------------------------------------------------
 * [엔진 파일 분할 가이드라인]
 *
 * [Level 1: Foundation]
 *  - Types.h: Vertex, ConstantBuffer 등 전역 구조체
 *  - Framework.h: DX11 라이브러리 및 Windows 헤더 모음
 *
 * [Level 2: Core Context]
 *  - GraphicsContext.h / .cpp: 렌더링 장치 담당
 *  - WindowContext.h / .cpp: 윈도우 생성 담당
 *
 * [Level 3: Resources]
 *  - Mesh.h / .cpp: 기하학적 데이터
 *  - Material.h / .cpp: 셰이더 및 속성 데이터 (추상/구체 클래스 분리 가능)
 *
 * [Level 4: Scene System]
 *  - Component.h: 컴포넌트 베이스
 *  - GameObject.h / .cpp: 엔티티 관리
 *  - MeshRenderer.h / .cpp: 실질적 렌더링 수행자
 *
 * [Level 5: Entry]
 *  - GameLoop.h / .cpp: 전체 조율
 *  - WinMain.cpp: 게임 콘텐츠 구성 및 실행
 * --------------------------------------------------------------------------------------
 */

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#include "GameLoop.hpp"
#include "MeshRenderer.hpp"

 // -----------------------------------------------------------------------------
 // [플레이어 컨트롤러 컴포넌트]
 // - 키보드 입력을 받아 별을 조종하는 예시 컴포넌트
 // -----------------------------------------------------------------------------
class PlayerController : public Component {
public:
    void Start(GraphicsContext* gfx) override {}

    void Input() override {} // GameLoop에서 전역 처리를 하므로 여기선 생략 가능

    void Update(float dt) override {
        float speed = 2.0f;
        if (GetAsyncKeyState(VK_UP) & 0x8000)    pOwner->pos.y += speed * dt;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000)  pOwner->pos.y -= speed * dt;
        if (GetAsyncKeyState(VK_LEFT) & 0x8000)  pOwner->pos.x -= speed * dt;
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) pOwner->pos.x += speed * dt;

        // 회전 효과
        pOwner->rot.z += dt;
    }

    void Render(GraphicsContext* gfx) override {}
};

// -----------------------------------------------------------------------------
// [윈도우 메시지 처리기]
// -----------------------------------------------------------------------------
LRESULT CALLBACK GlobalWndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProc(h, m, w, l);
}

// -----------------------------------------------------------------------------
// [메인 엔트리 포인트]
// -----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hI, HINSTANCE, LPSTR, int nS) 
{
    // 1. 엔진 매니저 초기화
    GameLoop gEngine;
    gEngine.Initialize(hI, GlobalWndProc);

    D3D11_INPUT_ELEMENT_DESC ied[] = 
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // 셰이더 컴파일 및 생성
    ShaderSet starShaders = gEngine.gfx.CompileAndCreate(L"effect.hlsl", 0, true, ied, 2);

    // 3. 메쉬 데이터 생성 (별 모양)
    float outerR = 0.5f;
    float innerR = 0.2f;
    XMFLOAT3 p[10];
    for (int i = 0; i < 10; ++i) 
    {
        float r = (i % 2 == 0) ? outerR : innerR;
        float angle = XM_PIDIV2 - (i * XM_2PI / 10.0f);
        p[i] = { cosf(angle) * r, sinf(angle) * r, 0.0f };
    }

    std::vector<Vertex> vStar;
    for (int i = 0; i < 10; i++) {
        vStar.push_back({ {0,0,0}, {1,1,1,1} }); // 중심점
        vStar.push_back({ p[i], {1,1,1,1} });
        vStar.push_back({ p[(i + 1) % 10], {1,1,1,1} });
    }

    Mesh* gMesh = new Mesh();
    gMesh->Create(&gEngine.gfx, vStar);

    // 4. 머티리얼 생성
    ColorMaterial* goldMat = new ColorMaterial(starShaders, { 1, 0.8f, 0, 1 }, gEngine.gfx.Device);
    ColorMaterial* redMat = new ColorMaterial(starShaders, { 1, 0.1f, 0.1f, 1 }, gEngine.gfx.Device);

    // 5. 난수를 이용한 다수의 별 생성
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disPos(-1.0f, 1.0f);
    std::uniform_real_distribution<float> disScale(0.1f, 0.3f);

    for (int i = 0; i < 20; i++) {
        GameObject* star = new GameObject(disPos(gen), disPos(gen), 0);
        star->scale = { disScale(gen), disScale(gen), 1.0f };

        // 렌더러와 컨트롤러 장착
        Material* selectedMat = (i % 2 == 0) ? (Material*)goldMat : (Material*)redMat;
        star->AddComponent(new MeshRenderer(gMesh, selectedMat));
        star->AddComponent(new PlayerController());

        gEngine.world.push_back(star);
    }

    // 6. 엔진 실행 (메인 루프)
    gEngine.Run();

    // 7. 자원 해제
    // 머티리얼 해제
    delete goldMat;
    delete redMat;

    // 셰이더 세트 해제
    starShaders.Release();

    // 메쉬 해제
    delete gMesh;

    // gEngine은 소멸자에서 world 내의 모든 GameObject를 delete함
    return 0;
}