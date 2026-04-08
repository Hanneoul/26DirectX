/*
* 
================================================================================
 [강의 노트: 싱글턴 패턴 (Singleton Pattern)]
================================================================================

 1. 싱글턴이란 무엇인가? (Definition)
    - 어떠한 클래스가 "오직 하나의 인스턴스(객체)"만 가지도록 보장하는 패턴임.
    - 어디서든지(Global) 이 단 하나의 객체에 접근할 수 있는 통로를 제공함.

 2. 왜 생겨났는가? (History & Origin)
    - 1994년 'GoF(Gang of Four)의 디자인 패턴' 책에서 처음 공식화됨.
    - 프로그램 전체에서 공유해야 하는 '자원 관리자'나 '설정값'이 중복 생성되어 
      데이터가 꼬이는 것을 방지하기 위해 고안됨.

 3. 왜 쓰는가? (Motivation)
    - [자원 공유]: 게임 엔진의 렌더러, 머티리얼 매니저 등은 하나만 존재해야 함.
    - [메모리 절약]: 똑같은 관리 객체를 여러 개 만들 필요가 없음.
    - [데이터 일관성]: 전역에서 동일한 상태(State)를 유지하기 쉬움.

 4. 시스템에서 왜 필요한가? (Engine Context)
    - 머티리얼 시스템에서 수천 개의 오브젝트가 "나 금색 머티리얼 줘!"라고 요청할 때,
      중앙 관리자(싱글턴)가 없으면 각자 파일을 읽어서 메모리가 터질 수 있음.
    - '단 하나의 창구'를 만들어 관리 효율을 극대화함.
================================================================================
* 
================================================================================
 [시스템 자원 관리를 위한 싱글턴]
================================================================================
 1. 필요성:
    - 윈도우 핸들(HWND)이나 DX11 디바이스 컨텍스트는 프로그램 전체에서 딱 하나임.
    - 여러 함수에서 이 정보를 쓰려면 인자로 계속 넘겨줘야 하는데(매우 번거로움),
      싱글턴을 쓰면 어디서든 'VideoSystem::GetInstance()->GetHWND()'로 접근 가능함.

 2. 유니티와의 비교:
    - 유니티의 'Screen' 클래스나 'QualitySettings'도 내부적으로는 이런 식으로
      시스템 자원을 관리하는 싱글턴 구조로 되어 있음.
================================================================================
*/

#include <windows.h>
#include <d3d11.h>
#include <iostream>

class VideoSystem {
private:
    // --- [클래식 싱글턴의 필수 요소] ---
    static VideoSystem* m_pInstance;

    VideoSystem() : m_hWnd(nullptr), m_pDevice(nullptr), m_pContext(nullptr) {
        std::cout << "VideoSystem: 시스템 객체가 생성됨.\n";
    }

    // 복사 방지
    VideoSystem(const VideoSystem&) = delete;
    VideoSystem& operator=(const VideoSystem&) = delete;

    // --- [시스템 리소스들] ---
    HWND m_hWnd;
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pContext;

public:
    static VideoSystem* GetInstance() {
        if (m_pInstance == nullptr) {
            m_pInstance = new VideoSystem();
        }
        return m_pInstance;
    }

    // 시스템 자원 등록 (초기화 시 1회 호출)
    void Initialize(HWND hwnd, ID3D11Device* dev, ID3D11DeviceContext* ctx) {
        m_hWnd = hwnd;
        m_pDevice = dev;
        m_pContext = ctx;
    }

    // 어디서든 자원을 꺼내 쓸 수 있는 Getter들
    HWND GetHWND() const { return m_hWnd; }
    ID3D11Device* GetDevice() const { return m_pDevice; }
    ID3D11DeviceContext* GetContext() const { return m_pContext; }

    static void Release() {
        if (m_pInstance) {
            delete m_pInstance;
            m_pInstance = nullptr;
            std::cout << "VideoSystem: 시스템 객체가 해제됨.\n";
        }
    }
};

// static 초기화
VideoSystem* VideoSystem::m_pInstance = nullptr;

// --- [사용 예시] ---

// 어느 깊은 곳에 있는 함수라고 가정함
void DrawSomething() {
    // 인자로 넘겨받지 않아도 싱글턴을 통해 바로 컨텍스트를 가져옴
    ID3D11DeviceContext* pCtx = VideoSystem::GetInstance()->GetContext();

    if (pCtx) {
        std::cout << "함수: 싱글턴을 통해 VideoContext에 접근하여 그리기 명령을 내림.\n";
    }
}

int main() {
    // 1. 초기화 (WinMain이나 DX 초기화 루틴에서 실행)
    // 실제로는 유효한 핸들과 디바이스를 넣어야 함
    VideoSystem::GetInstance()->Initialize((HWND)0x1234, nullptr, (ID3D11DeviceContext*)0x5678);

    // 2. 로직 수행
    DrawSomething();

    // 3. 종료
    VideoSystem::Release();

    return 0;
}