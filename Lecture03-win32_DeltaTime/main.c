#include <windows.h>
#include <stdio.h>

/*
 * [포인트 1: 왜 QueryPerformanceCounter인가?]
 * - 일반적인 time()이나 GetTickCount()는 정밀도가 낮음 (약 10~16ms 단위).
 * - 게임은 1ms(0.001초) 이하의 정밀도가 필요하므로, CPU의 '클럭 진동수'를 직접 이용하는 고해상도 타이머를 씀.
 */

typedef struct {
    LARGE_INTEGER frequency; // 1초당 CPU가 진동하는 횟수 (고정값)
    LARGE_INTEGER prevTime;  // 이전 프레임의 시점 기록
    double deltaTime;        // 두 시점 사이의 시간 간격 (초 단위)
} CGameTimer;

// 타이머 초기화: 시스템의 성능 주파수를 먼저 알아내야 함.
void InitTimer(CGameTimer* timer) {
    // 시스템이 지원하는 고해상도 타이머의 주파수를 획득
    QueryPerformanceFrequency(&timer->frequency);
    // 현재 시점의 카운트 값을 초기값으로 저장
    QueryPerformanceCounter(&timer->prevTime);
    timer->deltaTime = 0.0;
}

// 델타 타임 업데이트: 매 루프(프레임)마다 호출함.
void UpdateTimer(CGameTimer* timer) {
    LARGE_INTEGER currentTime;
    // 1. 현재 시점의 카운트 값을 가져옴
    QueryPerformanceCounter(&currentTime);

    /* * [포인트 2: 델타 타임 계산 공식]
     * (현재 카운트 - 이전 카운트) = 흐른 진동 횟수
     * (흐른 진동 횟수 / 초당 진동 횟수) = 흐른 시간(초)
     */
    timer->deltaTime = (double)(currentTime.QuadPart - timer->prevTime.QuadPart) / (double)timer->frequency.QuadPart;

    // 2. 다음 프레임 계산을 위해 현재 시간을 '이전 시간'으로 갱신
    timer->prevTime = currentTime;
}

int main() {
    CGameTimer myTimer;
    InitTimer(&myTimer);

    printf("C 스타일 고해상도 타이머 시작 (Ctrl+C로 종료)\n");

    int i = 0;
    for (i = 0; i < 10; ++i) {
        UpdateTimer(&myTimer);
        
        printf("DeltaTime: %f sec (FPS: %f)\n", myTimer.deltaTime, 1.0 / myTimer.deltaTime);
       
        Sleep(100);
        // 실제 게임 루프라면 여기서 playerPos += speed * myTimer.deltaTime; 로직이 들어감.
    }
    return 0;
}