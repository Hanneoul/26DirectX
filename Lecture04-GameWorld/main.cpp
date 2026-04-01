/*
================================================================================
 [가이드: 게임 엔진의 뼈대 만들기]
================================================================================
 1. Component (기능): 캐릭터가 할 수 있는 '일' (이동, 시간 재기 등)
 2. GameObject (객체): 게임에 존재하는 '물체' (플레이어, 타이머 등)
 3. GameWorld (세계): 모든 물체를 담고 있는 '바구니'

 * 구조: Component -> GameObject -> GameWorld 순으로 확장됨.
         (루프 한 번 돌 때 [입력 -> 업데이트 -> 렌더링] 순서로 모든 객체를 훑음.)
 [작동 원리]
 - Start(): 물체가 태어날 때 딱 한 번 실행되는 초기화 코드
 - OnInput(): 키보드/마우스 상태를 확인.
 - OnUpdate(): 수치(좌표 등)를 계산.
 - OnRender(): 화면에 결과를 출력.

================================================================================
*/

#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <vector>
#include <string>

// [1단계: 컴포넌트 기저 클래스]
// 모든 기능(이동, 렌더링 등)은 이 클래스를 상속받아야 함.
class Component {
public:
    class GameObject* pOwner; // 이 기능이 누구의 것인지 저장
    bool isStarted;           // Start()가 실행되었는지 체크

    virtual void Start() = 0;              // 초기화
    virtual void OnInput() {}              // 입력 (선택사항)
    virtual void OnUpdate(float dt) = 0;    // 로직 (필수)
    virtual void OnRender() {}             // 그리기 (선택사항)

    virtual ~Component() {}
};

// [2단계: 게임 오브젝트 클래스]
// 컴포넌트들을 담는 바구니 역할을 함.
class GameObject {
public:
    std::string name;
    std::vector<Component*> components;

    GameObject(std::string n) {
        name = n;
    }

    // 객체가 죽을 때 담고 있던 컴포넌트들도 메모리에서 해제함
    ~GameObject() {
        for (int i = 0; i < (int)components.size(); i++) {
            delete components[i];
        }
    }

    // 새로운 기능을 추가하는 함수
    void AddComponent(Component* pComp) {
        pComp->pOwner = this;
        pComp->isStarted = false;
        components.push_back(pComp);
    }
};

// --- [3단계: 실제 구현할 기능 컴포넌트들] ---

// 기능 1: 플레이어 조종 및 이동
class PlayerControl : public Component {
public:
    float x, y, speed;
    bool moveUp, moveDown, moveLeft, moveRight;

    void Start() override {
        x = 50.0f; y = 50.0f; speed = 150.0f;
        moveUp = moveDown = moveLeft = moveRight = false;
        printf("[%s] PlayerControl 기능 시작!\n", pOwner->name.c_str());
    }

    // [입력 단계] 키 상태만 체크함
    void OnInput() override {
        moveUp = (GetAsyncKeyState('W') & 0x8000);
        moveDown = (GetAsyncKeyState('S') & 0x8000);
        moveLeft = (GetAsyncKeyState('A') & 0x8000);
        moveRight = (GetAsyncKeyState('D') & 0x8000);
    }

    // [업데이트 단계] 체크된 키 상태로 좌표만 계산함
    void OnUpdate(float dt) override {
        if (moveUp)    y -= speed * dt;
        if (moveDown)  y += speed * dt;
        if (moveLeft)  x -= speed * dt;
        if (moveRight) x += speed * dt;
    }

    // [렌더링 단계] 계산된 좌표를 화면에 그림
    void OnRender() override {
        // 실제 엔진이라면 여기서 DirectX Draw를 부름
        // 지금은 좌표 시각화로 대체
        int py = (int)(y / 15.0f);
        int px = (int)(x / 10.0f);
        for (int i = 0; i < py; i++) printf("\n");
        for (int i = 0; i < px; i++) printf(" ");
        printf("★");
    }
};

// 기능 2: 시스템 정보 출력 (위치 정보 없음)
class InfoDisplay : public Component {
public:
    float totalTime;

    void Start() override {
        totalTime = 0.0f;
        printf("[%s] InfoDisplay 기능 시작!\n", pOwner->name.c_str());
    }

    void OnUpdate(float dt) override {
        totalTime += dt;
    }

    void OnRender() override {
        // 화면 최상단에 정보 출력
        printf("System Time: %.2f sec\n", totalTime);
        printf("Control: W, A, S, D | Exit: ESC\n");
    }
};

// 콘솔의 특정 좌표로 커서를 이동시키는 함수
void MoveCursor(int x, int y) {
    // \033[y;xH  (y와 x는 1부터 시작함)
    printf("\033[%d;%dH", y, x);
}

// 화면 전체를 지우는 함수 (루프 시작 전 한 번 사용)
void ClearScreen() {
    printf("\033[2J");
}

// --- [4단계: 메인 엔진 루프] ---
int main() {
    // [초기화] 게임 월드와 객체 생성
    std::vector<GameObject*> gameWorld;

    // 시스템 정보 객체 조립
    GameObject* sysInfo = new GameObject("SystemManager");
    InfoDisplay* pInfo = new InfoDisplay();
    sysInfo->AddComponent(pInfo);
    gameWorld.push_back(sysInfo);

    // 플레이어 객체 조립
    GameObject* player = new GameObject("Player1");
    PlayerControl* pControl = new PlayerControl();
    player->AddComponent(pControl);
    gameWorld.push_back(player);

    

    // 시간 측정 준비
    std::chrono::high_resolution_clock::time_point prevTime = std::chrono::high_resolution_clock::now();

    // --- [무한 게임 루프] ---
    while (true) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

        // A. 시간 관리 (DeltaTime 계산)
        std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - prevTime;
        float dt = elapsed.count();
        prevTime = currentTime;

        // ?. 스타트 실행
        for (int i = 0; i < (int)gameWorld.size(); i++) 
        {
            for (int j = 0; j < (int)gameWorld[i]->components.size(); j++) 
            {
                // Start()가 호출된 적 없다면 여기서 호출 (유니티 방식)
                if (gameWorld[i]->components[j]->isStarted == false)
                {
                    gameWorld[i]->components[j]->Start();
                    gameWorld[i]->components[j]->isStarted = true;
                }
            }
        }

        // B. 입력 단계 (Input Phase)
        for (int i = 0; i < (int)gameWorld.size(); i++) {
            for (int j = 0; j < (int)gameWorld[i]->components.size(); j++) {
                gameWorld[i]->components[j]->OnInput();
            }
        }

        // C. 업데이트 단계 (Update Phase)
        for (int i = 0; i < (int)gameWorld.size(); i++) 
        {
            for (int j = 0; j < (int)gameWorld[i]->components.size(); j++) 
            {
                gameWorld[i]->components[j]->OnUpdate(dt);
            }
        }

        // D. 렌더링 단계 (Render Phase)
        system("cls"); // 화면 지우기
        for (int i = 0; i < (int)gameWorld.size(); i++) {
            for (int j = 0; j < (int)gameWorld[i]->components.size(); j++) {
                gameWorld[i]->components[j]->OnRender();
            }
        }

        // CPU 과부하 방지 (약 60~100 FPS 유지 시도)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // [정리] 메모리 해제
    for (int i = 0; i < (int)gameWorld.size(); i++) {
        delete gameWorld[i]; // GameObject 소멸자가 컴포넌트들도 지움
    }

    return 0;
}