## 게임 서버
게임서버 포트폴리오 서버 코드입니다. <br/>

**기술 및 환경** : C++, IOCP, MSSQL, Windows<br/>
**컨텐츠 구현** : 로그인, 캐릭터 생성, 인벤토리(아이템), 몬스터 AI<br/>
**영상(유튜브)** : https://www.youtube.com/watch?v=S3F_YfWpQFE<br/>
**게임 클라이언트** : https://github.com/seejh/UnrealClient<br/>
**PPT** : https://docs.google.com/presentation/d/e/2PACX-1vTtF1DSAc5PI5ahFQDwn1GDq-nnUYIgpfZc4JpvFfdC26cMdELmuquaj6gUC54CHBmDTh0cXCBKEkHr/embed?start=false&loop=false&delayms=3000<br/>

**프로그램 설명**<br/>
![image](https://github.com/seejh/ServerPortfolio/assets/152791315/fb1d0163-3a41-4ad5-a077-5162a11ef021)
<br/>

**기본 동작**<br/>
IOCP, 잡, Send 스레드로 동작합니다.<br/>
클라이언트로부터 패킷이 오면 IOCP 스레드에서 받고 작업에 따라 IOCP 스레드에서 처리하던가 잡 스레드로 넘겨서 패킷을 처리 후 
보낼 패킷을 적재, Send 스레드에서 모인 패킷들을 전송합니다.

**멀티스레딩**<br/>
thread local storage, 잡큐, 잡타이머

**메모리 관리**<br/>
메모리 풀, 오브젝트 풀, StlAllocator

**DB**<br/>
ODBC래핑, 저장프로시저를 사용하여 DB에 쿼리

**패킷**<br/>
구글 프로토버프 직렬화<br/>
패킷 생성 툴을 사용하여 패킷 등록

**그 외**<br/>


