#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <vector>
#include <string>

using namespace std;

class Component {
public:
    class GameObject* pOwner = nullptr;
    bool isStarted = false;

    virtual void Start()[(A)]
    virtual void Input() {}
    virtual void Update(float dt) = 0;
    virtual void Render() {}
    virtual ~Component() {}
};

class GameObject {
public:
    string name;
    vector<Component*> components;

    GameObject(string n) : name(n) {}
    ~GameObject() {
        // 인덱스 기반으로 컴포넌트 해제
        for (int i = 0; i < (int)components.size(); i++) {
            delete components[i];
        }
    }

    void AddComponent(Component* pComp) {
        pComp->pOwner = this;
        components.push_back(pComp);
    }
};

class GravityBody : public Component {
public:
    float y = 0.0f;
    float velocityY = 0.0f;

    void Start() override { y = 10.0f; velocityY = 0.0f; }
    void Update(float dt) override {
        velocityY += 9.8f * dt;
        [(B)]
    }
    void Render() override {
        printf("Object [%s] Height: %.2f\n", pOwner->name.c_str(), y);
    }
};

class GameLoop {
public:
    bool isRunning = true;
    vector<GameObject*> gameWorld;
    chrono::high_resolution_clock::time_point prevTime;

    void Initialize() {
        prevTime = chrono::high_resolution_clock::now();
    }

    void Run() {
        while ([(C)]) {
            // 시간 계산
            chrono::high_resolution_clock::time_point currentTime = chrono::high_resolution_clock::now();
            chrono::duration<float> elapsed = currentTime - prevTime;
            float dt = elapsed.count();
            prevTime = currentTime;

            // 1. Start 호출 체크 (지연 초기화)
            for (int i = 0; i < (int)gameWorld.size(); i++) {
                GameObject* go = gameWorld[i];
                for (int j = 0; j < (int)go->components.size(); j++) {
                    Component* c = go->components[j];
                    if (c->isStarted == false) {
                        c->Start();
                        c->isStarted = true;
                    }
                }
            }

            // 2. Input 단계
            for (int i = 0; i < (int)gameWorld.size(); i++) {
                GameObject* go = gameWorld[i];
                for (int j = 0; j < (int)go->components.size(); j++) {
                    go->components[j]->Input();
                }
            }

            // 3. Update 단계
            for (int i = 0; i < (int)gameWorld.size(); i++) {
                GameObject* go = gameWorld[i];
                for (int j = 0; j < (int)go->components.size(); j++) {
                    go->components[j]->Update(dt);
                }
            }

            // 4. Render 단계
            system("cls");
            for (int i = 0; i < (int)gameWorld.size(); i++) {
                GameObject* go = gameWorld[i];
                for (int j = 0; j < (int)go->components.size(); j++) {
                    go->components[j]->Render();
                }
            }

            if (GetAsyncKeyState(VK_ESCAPE)) isRunning = false;
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
};

int main() {
    GameLoop engine;
    engine.Initialize();

    GameObject* ball = new GameObject("Ball");
    ball->AddComponent(new GravityBody());
    engine.gameWorld.push_back(ball);

    engine.Run();

    // 메모리 정리 (시험 문제용으로 남겨둠)
    for (int i = 0; i < (int)engine.gameWorld.size(); i++) {
        delete engine.gameWorld[i];
    }

    return 0;
}