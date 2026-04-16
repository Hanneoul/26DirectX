1.Component 클래스의 Start() 함수(20번줄)를 자식 클래스에서 반드시 구현하도록 강제하고자 함. 20번째줄을 변경해 보시오.

2. deltatime측정을 위해 GameLoop의 currentTime이란 변수를 업데이트 해주는
currentTime=high_resolution_clock::now();
는 몇번째 줄이 실행되기 전에 수행되어야 할까요?

3. GetAsyncKeyState 를 활용해서 Update() 함수를 작성하려고 한다. 
   Input()함수에서 사용한 moveUp, moveDown, moveLeft, moveRight 를 활용하여 좌표를 업데이트하여 모니터의 프레임률에 독립적으로 움직이도록 x,y를 업데이트하는 코드를 작성하시오.
   (speed 변수를 흘러간 시간만큼 곱하여 이동치를 산정해주세요.)

if (moveUp)    y -= speed * dt;
if (moveDown)  y += speed * dt;
if (moveLeft)  x -= speed * dt;
if (moveRight) x += speed * dt;

4. 이 코드를 실행하면 실행즉시 종료되는 버그가 발생했습니다. 버그의 원인에 대해서 서술하세요.
(몇번줄에 어떤이유로 버그가 생겼다고 적어주세요)

5. 메모리 누수가 발생합니다. GameObject가 파괴될 때 자신이 가진 모든 컴포넌트의 메모리도 함께 해제하도록 GameObject 소멸자를 작성하세요.


