// MyServerPF.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include"pch.h"
#include"NetService.h"

#include"ClientSession.h"
#include"DBBind.h"
#include"DBConnectionPool.h"
#include"ClientPacketHandler.h"

#include"SessionManager.h"
#include"Room.h"
#include"Actor.h"
#include<bitset>

#include<map>

void DBSetting() {
    // connect db
    ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Database=MyDB;Trusted_Connection=Yes;"));
    
    // create DB
    {
        auto query = L"										    \
			DROP TABLE IF EXISTS [dbo].[Player];			\
			CREATE TABLE[dbo].[Player]						\
			(												\
				[id] INT NOT NULL PRIMARY KEY IDENTITY,		\
                [playerId] VARCHAR(20) NULL,               \
                [playerPw] VARCHAR(20) NULL,               \
				[win] INT NULL,								\
                [lose] INT NULL                             \
			);";
        DBConnection* dbConn = GDBConnectionPool->Pop();
        ASSERT_CRASH(dbConn->Execute(query));
        GDBConnectionPool->Push(dbConn);
    }

    // Add Player Data 1
    {
        auto query = L"INSERT INTO [dbo].[Player]([playerId], [playerPw]) VALUES(?, ?)";
        DBConnection* dbConn = GDBConnectionPool->Pop();
        DBBind<2, 0> dbBind(*dbConn, query);

        BYTE id[20] = "see";
        dbBind.BindParam(0, id);

        BYTE pw[20] = "aaa";
        dbBind.BindParam(1, pw);
        
        /*int win = 5;
        dbBind.BindParam(2, win);

        int lose = 10;
        dbBind.BindParam(3, lose);*/

        ASSERT_CRASH(dbBind.Execute());
        GDBConnectionPool->Push(dbConn);
    }

    // Add Player Data 2
    {
        auto query = L"INSERT INTO [dbo].[Player]([playerId], [playerPw]) VALUES(?, ?)";
        DBConnection* dbConn = GDBConnectionPool->Pop();
        DBBind<2, 0> dbBind(*dbConn, query);

        BYTE id[20] = "lee";
        dbBind.BindParam(0, id);

        BYTE pw[20] = "bbb";
        dbBind.BindParam(1, pw);

        ASSERT_CRASH(dbBind.Execute());
        GDBConnectionPool->Push(dbConn);
    }

    // Add Player Data 3
    {
        auto query = L"INSERT INTO [dbo].[Player]([playerId], [playerPw]) VALUES(?, ?)";
        DBConnection* dbConn = GDBConnectionPool->Pop();
        DBBind<2, 0> dbBind(*dbConn, query);

        BYTE id[20] = "kim";
        dbBind.BindParam(0, id);

        BYTE pw[20] = "ccc";
        dbBind.BindParam(1, pw);

        ASSERT_CRASH(dbBind.Execute());
        GDBConnectionPool->Push(dbConn);
    }
}

void IOCPTask(NetService* service) {
    while (true) {
        service->_iocpCore->Dispatch();
    }
}

int main()
{
    SessionManager* sessionManager = SessionManager::Instance();
    Room* globalRoom = Room::Instance();

    DBSetting();

    ClientPacketHandler::Init();

    // MAIN
    NetService* service = new NetService(make_shared<ClientSession>);
    service->Init();
    
    //
    thread iocpThread(IOCPTask, service);

    // TEST
    // Monster Setting
    //Room::Instance()->MonsterAdd();
    {
        shared_ptr<Monster> monster = make_shared<Monster>();
        monster->SetIndex(g_monsterIndexGenerator.fetch_add(1));
        monster->SetLocation(3240.0f, -30.0f, 143.0f);
        monster->SetRotation(-180.0f, 0, 0);
        monster->SetDamage(2);
        monster->SetHP(1000);

        Room::Instance()->MonsterAdd(monster);

        PROTOCOL::S_ENTER_ROOM pkt;
        pkt.mutable_actor()->set_index(monster->GetIndex());
        pkt.mutable_actor()->set_actortype(PROTOCOL::MONSTER);
        pkt.mutable_actor()->set_locationx(monster->GetLocation()._x);
        pkt.mutable_actor()->set_locationy(monster->GetLocation()._y);
        pkt.mutable_actor()->set_locationz(monster->GetLocation()._z);
        pkt.mutable_actor()->set_rotationyaw(monster->GetRotation()._yaw);
        pkt.mutable_actor()->set_rotationpitch(monster->GetRotation()._pitch);
        pkt.mutable_actor()->set_rotationroll(monster->GetRotation()._roll);
        pkt.mutable_actor()->set_hp(monster->GetHP());

        auto sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);
        Room::Instance()->Broadcast(sendBuffer);
    }

    // Monster Thread
    cout << "MonsterThread Run" << endl;
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(150));
        Room::Instance()->Update();
    }

    iocpThread.join();
}
