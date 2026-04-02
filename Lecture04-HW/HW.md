DirectX 11: 컴포넌트 기반 게임 오브젝트 시스템 구현

* 과제 개요
본 과제는 Lecture04-GameWorld에서 학습한 게임 루프(Game Loop) 및 컴포넌트 패턴(Component Pattern)의 핵심 개념을 실제 코드로 구현하는 능력을 검증해보는게 목표. 
학생들은 Win32 API와 C++ 기반의 DirectX 11 환경에서 프레임 독립적인 객체 이동과 구조적인 엔진 설계 방식을 본인이 익혔는지 확인해본다.

* 개발 환경 및 기술 제약
프로그래밍 언어: C++ (Modern C++ 권장)
프레임워크/API: Win32 API 및 DirectX 11 (D3D11)
화면 설정: 해상도 800 * 600 (Fixed Size)
핵심 설계: GameLoop, GameObject와 Component 클래스 구조를 반드시 Lecture04-GameWorld와 같이 적용할 것

* 세부 구현 요구사항
A. 게임 엔진 구조 (Game Engine Architecture)
	- 프레임 독립적 이동 (DeltaTime):
		- PeekMessage 기반의 Non-blocking 게임 루프를 구축하십시오.
		- 고해상도 타이머를 사용하여 프레임 간 시간 간격인 DeltaTime(dt)을 초 단위로 계산하십시오.
	- 모든 객체의 이동은 반드시 다음의 공식을 따라야 합니다:
		- Position = Position + (Velocity * DeltaTime)
	- 객체 및 컴포넌트 설계 (GameObject/Component):
		- GameObject 클래스는 위치(Position) 정보를 가지며, 부착된 Component들의 Update와 Render를 일괄 호출해야 합니다.
		- 추상 클래스 Component를 설계하고, 이를 상속받아 삼각형을 그리는 Renderer 컴포넌트를 구현하십시오.

B. 과제물 기능 구현 (Functional Requirements)
	- 삼각형 GameObject 생성:
		- 서로 다른 색상을 가진 두 개의 삼각형을 생성하고, 각각 독립적인 GameObject 인스턴스로 관리하십시오.
	- 개별 조작 시스템:
		- 삼각형 1 (Player 1): 방향키(상, 하, 좌, 우)를 이용하여 이동합니다.
		- 삼각형 2 (Player 2): W, A, S, D 키를 이용하여 상하좌우로 이동합니다.
	- 시스템 제어 기능:
		- ESC 키: 프로그램 즉시 종료 및 관련 메모리 해제.
		- F 키: 창 모드(Windowed)와 전체 화면(Full Screen) 모드를 전환(Toggle)하는 기능을 구현하십시오.