#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <vector>
#include <string>

void MoveCursor(int x, int y)
{
    printf("\033[%d;%dH", y, x);
}

class Component
{
public:
    class GameObject* pOwner = nullptr;
    bool isStarted = false;

    // 1번문제
    virtual void Start();
    virtual void Update(float dt);
    virtual void Input();
    virtual void Render();
    virtual ~Component();
};

class GameObject {
public:
    std::string name;
    std::vector<Component*> components;

    GameObject(std::string n)
    {
        name = n;
    }

    ~GameObject() 
    {
        //4번문제 버그발생지점
    }

    void AddComponent(Component* pComp)
    {
        pComp->pOwner = this;
        pComp->isStarted = false;
        components.push_back(pComp);
    }
};

class PlayerControl : public Component {
public:
    float x, y, speed;
    bool moveUp, moveDown, moveLeft, moveRight;

    void Start() override
    {
        x = 50.0f; y = 50.0f; speed = 150.0f;
        moveUp = moveDown = moveLeft = moveRight = false;
    }

    void Input() override
    {
        moveUp = (GetAsyncKeyState('W') & 0x8000);
        moveDown = (GetAsyncKeyState('S') & 0x8000);
        moveLeft = (GetAsyncKeyState('A') & 0x8000);
        moveRight = (GetAsyncKeyState('D') & 0x8000);
    }

    void Update(float dt) override
    {
        if (moveUp)    y -= speed * dt;
        if (moveDown)  y += speed * dt;
        if (moveLeft)  x -= speed * dt;
        if (moveRight) x += speed * dt;
    }

    void Render() override
    {
        int px = (int)(x / 10.0f);
        int py = (int)(y / 15.0f);
        MoveCursor(px, py);
        printf("★");
    }
};

class GameLoop {
public:
    bool isRunning;
    std::vector<GameObject*> gameWorld;
    std::chrono::high_resolution_clock::time_point currentTime;
    std::chrono::high_resolution_clock::time_point prevTime;
    float deltaTime;

    void Initialize()
    {
        isRunning = true;
        gameWorld.clear();
        prevTime = std::chrono::high_resolution_clock::now();
        deltaTime = 0.0f;
    }

    void Run()
    {
        while (!isRunning) 
        {
            std::chrono::duration<float> elapsed = currentTime - prevTime;
            deltaTime = elapsed.count();
            prevTime = currentTime;

            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) isRunning = false;

            for (int i = 0; i < (int)gameWorld.size(); i++)
            {
                for (int j = 0; j < (int)gameWorld[i]->components.size(); j++)
                {
                    Component* comp = gameWorld[i]->components[j];
                    if (comp->isStarted == false)
                    {
                        comp->Start();
                        comp->isStarted = true;
                    }
                    comp->Input();
                    comp->Update(deltaTime);
                }
            }

            system("cls");
            for (int i = 0; i < (int)gameWorld.size(); i++)
            {
                for (int j = 0; j < (int)gameWorld[i]->components.size(); j++)
                {
                    gameWorld[i]->components[j]->Render();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    ~GameLoop() {
        for (int i = 0; i < (int)gameWorld.size(); i++)
        {
            delete gameWorld[i];
        }
    }
};

int main()
{
    GameLoop gLoop;
    gLoop.Initialize();

    GameObject* player = new GameObject("Player1");
    player->AddComponent(new PlayerControl());
    gLoop.gameWorld.push_back(player);

    gLoop.Run();

    return 0;
}