## 게임서버
게임서버 포트폴리오 서버측 코드이며 IOCP 기반 서버입니다. <br/>
로그인, 캐릭터 이동과 공격, 채팅, 몬스터가 구현되어 있으며 프로그램 동작의 큰 흐름은 아래와 같습니다.<br/>
![Server1](https://github.com/seejh/ServerPortfolio/assets/152791315/4bde2fb8-6eac-4b08-b257-fd72a6bd58d1)
IOCP로 클라이언트로부터 패킷과 몬스터AI처리를 큐에 적재하고 게임 업데이트 스레드에서 빼내어 처리합니다.

프로그램이 시작되면 몬스터를 만들고 클라이언트의 접속을 기다립니다. 클라이언트가 소켓으로 Connect하면 Session, 로그인하면 Player로써 관리되며
그 후 클라이언트가 방 입장 패킷을 보내고 룸(월드)에 접속하면 게임을 시작하게 됩니다. 
네트워크 패킷 직렬화는 구글 프로토버프로 했고 동기화는 std::mutex를 사용했으며 몬스터AI는 FSM으로 만들었습니다.
코드는 네트워크 라이브러리(MyServerCore)와 컨텐츠 코드(MyServerPF)로 분리되어 있습니다.



## 개발 환경
**서버** : Windows, C++, IOCP<br/>
**클라이언트** : Windows, Unreal4 <br/>
**DB** : MySQL<br/>

## 코드(깃허브)<br/>
**서버** : https://github.com/seejh/ServerPortfolio<br/>
**클라이언트** : https://github.com/seejh/ClientPortfolio<br/> 

## 영상(유튜브)<br/>
https://www.youtube.com/watch?v=A-Fom8oiGkY


----------------------------------------------------------------------------------------------------------
## 게임 서버
게임서버 포트폴리오 서버 코드입니다. <br/><br/>

**기술 및 환경** : C++, IOCP, MSSQL, Windows<br/>
**구현** : 로그인, 캐릭터 생성, 인벤토리(아이템), 몬스터 AI<br/><br/>

**프로그램 동작 설명**<br/>
크게 IOCP, 잡큐, Send 스레드로 나누어지고 기본적인 동작은 아래와 같습니다.<br/>
![image](https://github.com/seejh/ServerPortfolio/assets/152791315/fb1d0163-3a41-4ad5-a077-5162a11ef021)
<br/>
IOCP에서 클라로부터 패킷을 받고 여기서 처리하거나, 잡큐로 넘겨서 처리하고 Send 스레드에서 처리된 내용들을 클라이언트에게 보내줍니다.<br/>
잡큐는 게임월드(룸)와 DB에 대한 연산을 처리합니다.<br/>





