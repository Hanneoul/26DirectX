/*
 * ==============================================================================
 * [윈도우 입력 시스템과 OS I/O 모델]
 * ==============================================================================
 * 1. GetKeyState vs GetAsyncKeyState
 * - GetKeyState: '메시지 큐' 기반. 윈도우 메시지가 처리된 시점의 키 상태를 가져옴. (동기적)
 * - GetAsyncKeyState: 메시지 큐 무시. 호출한 '이 순간' 하드웨어의 실제 상태를 찌름. (비동기적)
 * - 게임 엔진에서는 인풋 렉을 최소화하기 위해 'Async' 버전을 선호함.
 *
 * 2. 왜 0x8000 (1000 0000 0000 0000) 인가?
 * - 리턴값은 16비트 정수(short)임.
 * - 최상위 비트(MSB): 현재 키가 눌려 있는가? (1 = 눌림, 0 = 안눌림)
 * - 최하위 비트(LSB): 지난 호출 이후 키가 한 번이라도 눌렸었는가? (신뢰도 낮음)
 * - 그래서 '& 0x8000' 연산을 통해 "다른 비트는 다 무시하고, '지금 눌림' 상태인지만 보겠다"는 뜻임.
 *
 * 3. 0x8001 같은 값은 왜 안 쓰나?
 * - 윈도우 버전마다 하위 비트 동작이 다름. 0x8000 마스킹이 가장 안전한 표준임.
 * ==============================================================================
 */

#include <windows.h>
#include <stdio.h>

 // 간단한 좌표 구조체
typedef struct 
{ 
    float x, y; 
}Point;

int main() {
    printf("=== [Input System Deep Dive] ===\n");
    printf("Control: W, A, S, D | Exit: ESC\n");
    printf("Check out the MSB (0x8000) logic in comments.\n\n");

    Point player = { 10.0f, 10.0f };
    int isRunning = 1;

    // --- [Real-time Game Loop] ---
    while (isRunning) {
        // 1. [Non-blocking Polling]
        // 메시지 큐를 기다리지 않고 OS 커널의 입력 버퍼 상태를 직접 읽음.
        short escState = GetAsyncKeyState(VK_ESCAPE);
        if (escState & 0x8000) isRunning = 0;

        // 2. [Async Input Processing]
        float speed = 0.5f;

        // 0x8000 비트가 켜져 있는지(1인지) 확인하여 현재 눌림 상태를 판단
        if (GetAsyncKeyState('W') & 0x8000) player.y -= speed;
        if (GetAsyncKeyState('S') & 0x8000) player.y += speed;
        if (GetAsyncKeyState('A') & 0x8000) player.x -= speed;
        if (GetAsyncKeyState('D') & 0x8000) player.x += speed;

        // 3. [Rendering - Console version]
        // 화면을 지우고 좌표에 플레이어(*) 출력 (실제 엔진의 Render 단계)
        system("cls");
        printf("Player Info: X:%.1f, Y:%.1f\n", player.x, player.y);
        printf("Raw 'A' State: 0x%04X\n", (unsigned short)GetAsyncKeyState('A'));

        // 좌표에 맞춰 빈칸 출력 (간이 렌더링)
        for (int i = 0; i < (int)player.y; i++) printf("\n");
        for (int i = 0; i < (int)player.x; i++) printf(" ");
        printf("★"); // 우리들의 육망성(별) 대신 별 문자 출력

        // 4. [Frame Synchronization]
        // 너무 빠르면 안 보이니 30ms 정도 쉬어줌 (OS에게 CPU를 양보하는 Non-blocking의 미덕)
        Sleep(30);
    }

    printf("\nGame Over. 모든 자원을 안전하게 반납했습니다.\n");
    return 0;
}