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
 - Input(): 키보드/마우스 상태를 확인.
 - Update(): 수치(좌표 등)를 계산.
 - Render(): 화면에 결과를 출력.
  
  
================================================================================
 [가이드2: 객체지향과 추상화]
================================================================================
 1. 왜 GameObject와 Component를 분리하는가? (Composition vs Inheritance)
  - 상속(Inheritance)의 한계: "플레이어는 캐릭터다", "NPC는 캐릭터다" 식의 설계는
    기능이 복합해질수록 계층 구조가 꼬임 (예: 날아다니는 상자? 캐릭터인가 아이템인가?)
  - 조립(Composition)의 장점: GameObject는 빈 껍데기일 뿐이며, 필요한 기능(Component)을
    마치 레고 블록처럼 끼워 맞추는 방식임. 유연성과 재사용성이 극대화됨.

 2. 순수 가상 함수(= 0)를 사용하는 설계적 이유
  - [강제성]: 부모 클래스(Component)는 "모든 기능은 반드시 실행(Update)되어야 한다"는
    규칙만 정의함. 실제 세부 내용은 자식(PlayerControl 등)이 결정함.
  - [추상 클래스]: Component 자체는 불완전한 클래스이므로 'new Component()'처럼
    단독 객체 생성을 문법적으로 막아버림. (실수 방지)

 3. 가상 함수({})와 가상 소멸자의 필요성
  - [선택적 확장]: Input()이나 Render()를 일반 가상 함수로 둔 이유는, 모든 객체가
    입력을 받거나 그려질 필요는 없기 때문임. (예: 보이지 않는 데이터 관리자)
  - [다형성 소멸]: GameObject가 Component* 포인터로 자식을 관리하므로,
    부모의 소멸자가 virtual이어야만 delete 시 실제 자식 객체의 메모리까지 해제됨.

 4. 실행 순서의 약속 (Lifecycle)
  - Start -> Input -> Update -> Render 순서의 규격화는 수천 개의 객체가
    서로 꼬이지 않고 예측 가능한 순서로 동작하게 만드는 엔진의 '철로' 역할을 함.
================================================================================
*/




#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <vector>
#include <string>

// 콘솔의 특정 좌표로 커서를 이동시키는 함수
void MoveCursor(int x, int y) 
{
    // \033[y;xH  (y와 x는 1부터 시작함)
    printf("\033[%d;%dH", y, x);
}




// [1단계: 컴포넌트 기저 클래스]
// 모든 기능(이동, 렌더링 등)은 이 클래스를 상속받아야 함.
class Component 
{
public:
    class GameObject* pOwner = nullptr; // 이 기능이 누구의 것인지 저장
    bool isStarted = 0;           // Start()가 실행되었는지 체크

    /*
     * 1. 순수 가상 함수 (Pure Virtual Function): = 0
     * - "이 기능은 자식 클래스에서 반드시! 무조건! 구현해야 함"을 의미함.
     * - 이유: 초기화(Start)나 로직(Update)은 컴포넌트마다 필수적인 동작이므로,
     *         설계자가 실수로 구현을 빼먹는 것을 컴파일 단계에서 방지하기 위함.
     * - 특징: 이 함수가 하나라도 구현 안 되면 객체 생성이 불가능함.
     */
    virtual void Start() = 0;              // 초기화
    virtual void Update(float dt) = 0;     // 로직 (필수)

    /*
     * 2. 일반 가상 함수 (Virtual Function): {}
     * - "기본 동작은 비어있으니, 필요하면 재정의(Override)해서 쓰라"는 의미임.
     * - 이유: 모든 컴포넌트가 입력을 받거나(Input) 화면에 그릴(Render) 필요는 없음.
     * (예: 점수 계산 컴포넌트는 화면에 그릴 필요가 없을 수 있음)
     * - 특징: 구현하지 않아도 컴파일 에러가 나지 않으며, 호출 시 아무 일도 일어나지 않음.
     */
    virtual void Input() {}                // 입력 (선택사항)
    virtual void Render() {}               // 그리기 (선택사항)

    // 가상 소멸자: 부모 포인터로 자식 객체를 삭제할 때, 자식의 소멸자까지 안전하게 호출하기 위함.
    virtual ~Component() {}
};

// [2단계: 게임 오브젝트 클래스]
// 컴포넌트들을 담는 바구니 역할을 함.
class GameObject {
public:
    std::string name;
    std::vector<Component*> components;

    GameObject(std::string n) 
    {
        name = n;
    }

    // 객체가 죽을 때 담고 있던 컴포넌트들도 메모리에서 해제함
    ~GameObject() {
        for (int i = 0; i < (int)components.size(); i++) 
        {
            delete components[i];
        }
    }

    // 새로운 기능을 추가하는 함수
    void AddComponent(Component* pComp) 
    {
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
    int playerType = 0;

    PlayerControl(int type)
    {
        playerType = type;
    }
    void Start() override 
    {
        x = 50.0f; y = 50.0f; speed = 150.0f;
        moveUp = moveDown = moveLeft = moveRight = false;        
        printf("[%s] PlayerControl 기능 시작!\n", pOwner->name.c_str());        
    }

    // [입력 단계] 키 상태만 체크함
    void Input() override 
    {
        if (playerType == 0)
        {
            moveUp = (GetAsyncKeyState('W') & 0x8000);
            moveDown = (GetAsyncKeyState('S') & 0x8000);
            moveLeft = (GetAsyncKeyState('A') & 0x8000);
            moveRight = (GetAsyncKeyState('D') & 0x8000);
        }
        if (playerType == 1)
        {
            moveUp = (GetAsyncKeyState(VK_UP) & 0x8000);
            moveDown = (GetAsyncKeyState(VK_DOWN) & 0x8000);
            moveLeft = (GetAsyncKeyState(VK_LEFT) & 0x8000);
            moveRight = (GetAsyncKeyState(VK_RIGHT) & 0x8000);
        }
    }

    // [업데이트 단계] 체크된 키 상태로 좌표만 계산함
    void Update(float dt) override 
    {
        if (moveUp)    y -= speed * dt;
        if (moveDown)  y += speed * dt;
        if (moveLeft)  x -= speed * dt;
        if (moveRight) x += speed * dt;
    }

    // [렌더링 단계] 계산된 좌표를 화면에 그림
    void Render() override 
    {
        // 실제 엔진이라면 여기서 DirectX Draw를 부름
        // 지금은 좌표 시각화로 대체
        
        if (x < 10.0f)
            x = 10.0f;
        if (y < 45.0f)
            y = 45.0f;

        int py = (int)(y / 15.0f);
        int px = (int)(x / 10.0f);

        
        MoveCursor(px, py);

        if (playerType == 0)
            printf("★");
        if (playerType == 1)
            printf("☆");
        
    }
};

// 기능 2: 시스템 정보 출력 (위치 정보 없음)
class InfoDisplay : public Component 
{
public:
    float totalTime = 0.0f;

    void Start() override 
    {
        totalTime = 0.0f;
        printf("[%s] InfoDisplay 기능 시작!\n", pOwner->name.c_str());
    }

    void Update(float dt) override 
    {
        totalTime += dt;
    }

    void Render() override {
        // 화면 최상단에 정보 출력
        MoveCursor(0, 0);
        printf("System Time: %.2f sec\n", totalTime);
        printf("Control: W, A, S, D | Exit: ESC\n");
    }
};



class GameLoop
{
public:
    bool isRunning;
    std::vector<GameObject*> gameWorld;
    std::chrono::high_resolution_clock::time_point prevTime;
    float deltaTime;   //delta time;

    //초기화
    void Initialize()
    {
        //초기화시 동작준비됨
        isRunning = true;

        gameWorld.clear();

        // 시간 측정 준비
        prevTime = std::chrono::high_resolution_clock::now();
        deltaTime = 0.0f;

        
    }    

    void Input()
    {
        // esc 누르면 종료
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) isRunning = false;

        // B. 입력 단계 (Input Phase)
        for (int i = 0; i < (int)gameWorld.size(); i++)
        {
            for (int j = 0; j < (int)gameWorld[i]->components.size(); j++)
            {
                gameWorld[i]->components[j]->Input();
            }
        }
    }

    void Update()
    {
        // C. 스타트 실행
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

        // D. 업데이트 단계 (Update Phase)
        for (int i = 0; i < (int)gameWorld.size(); i++)
        {
            for (int j = 0; j < (int)gameWorld[i]->components.size(); j++)
            {
                gameWorld[i]->components[j]->Update(deltaTime);
            }
        }
    }

    void Render()
    {
        // E. 렌더링 단계 (Render Phase)
        system("cls");
        for (int i = 0; i < (int)gameWorld.size(); i++)
        {
            for (int j = 0; j < (int)gameWorld[i]->components.size(); j++)
            {
                gameWorld[i]->components[j]->Render();
            }
        }
    }

    

    void Run()
    {
        // --- [무한 게임 루프] ---
        while (isRunning) {

            // A. 시간 관리 (DeltaTime 계산)
            std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = currentTime - prevTime;
            deltaTime = elapsed.count();
            prevTime = currentTime;

            Input();
            Update();
            Render();

            // CPU 과부하 방지 (약 60~100 FPS 유지 시도)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    GameLoop()
    {
        Initialize();
    }
    ~GameLoop()
    {
        // [정리] 메모리 해제
        for (int i = 0; i < (int)gameWorld.size(); i++)
        {
            delete gameWorld[i]; // GameObject 소멸자가 컴포넌트들도 지움
        }
    }

};

// --- [4단계: 메인 엔진 루프] ---
int main() 
{
    //게임루프
    GameLoop gLoop;
    gLoop.Initialize();

    // 시스템 정보 객체 조립
    GameObject* sysInfo = new GameObject("SystemManager");
    sysInfo->AddComponent(new InfoDisplay());
    gLoop.gameWorld.push_back(sysInfo);

    // 플레이어 객체 조립
    GameObject* player1 = new GameObject("Player1");
    player1->AddComponent(new PlayerControl(0));
    gLoop.gameWorld.push_back(player1);

    // 플레이어 객체 조립
    GameObject* player2 = new GameObject("Player2");
    player2->AddComponent(new PlayerControl(1));
    gLoop.gameWorld.push_back(player2);

    //게임루프 실행
    gLoop.Run();

    return 0;
}