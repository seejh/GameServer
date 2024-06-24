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
#include"GameObject.h"
#include"GlobalQueue.h"
#include"CoreGlobal.h"
#include"ThreadManager.h"
#include"CSVReader.h"

#include<bitset>
#include<map>

#include"GenProcedures.h"
#include"DBSynchronizer.h"
#include"DBDataModel.h"
#include"DBTransaction.h"
#include"RoomManager.h"
#include"SessionManager.h"

/*
    
*/

#pragma region Task

void IocpAndJobTask(NetService* service) {
    while (true) {
        // IOCP - 0 ~ 10ms
        service->_iocpCore->Dispatch(10);

        // 잡타이머큐 처리
        GJobTimer->Distribute(GetTickCount64());

        // 잡큐 처리
        shared_ptr<JobQueue> jobQueue = GGlobalQueue->Pop();
        if (jobQueue == nullptr)
            continue;

        jobQueue->Execute();
    }
}

void NetworkFlushTask() {
    uint64 sendTerm = 100;

    while (true) {
        uint64 now = GetTickCount64();

        SessionManager::Instance()->FlushSend();

        this_thread::sleep_for(chrono::milliseconds(sendTerm));
    }
}

#pragma endregion


// DB에 사전 설정할 정보를 업데이트할 필요가 있을 때
void DBDataSetOnce() {
    DBConnection* dbConn = GDBConnectionPool->Pop();

    // 계정DB
    /*
    {
        wstring wstr(L"see");

        // DBConnection UnBind
        SP::InsertAccount insertAccount(*dbConn);
        insertAccount.In_AccountName(wstr.c_str(), wstr.size());
        insertAccount.In_AccountPw(111);
        
        insertAccount.Execute();
    }

    {
        wstring wstr(L"lee");

        // DBConnection UnBind
        SP::InsertAccount insertAccount(*dbConn);
        insertAccount.In_AccountName(wstr.c_str(), wstr.size());
        insertAccount.In_AccountPw(222);

        insertAccount.Execute();
    }

    {
        wstring wstr(L"kim");

        // DBConnectino UnBind
        SP::InsertAccount insertAccount(*dbConn);
        insertAccount.In_AccountName(wstr.c_str(), wstr.size());
        insertAccount.In_AccountPw(333);

        insertAccount.Execute();
    }
    */

    // 플레이어DB
    /*
    {
        wstring wstr(L"see1");

        // DBConnecction UnBind
        SP::InsertPlayer insertPlayer(*dbConn);
        insertPlayer.In_AccountDbId(1);
        insertPlayer.In_PlayerName(wstr.c_str(), wstr.size());
        insertPlayer.In_Level(1);
        insertPlayer.In_TotalExp(0);
        insertPlayer.In_MaxHp(200);
        insertPlayer.In_Hp(200);
        insertPlayer.In_Damage(20);
        insertPlayer.In_LocationX(0);
        insertPlayer.In_LocationY(0);
        insertPlayer.In_LocationZ(120.0f);
        insertPlayer.Execute();
    }

    {
        wstring wstr(L"lee1");

        // DBConnection UnBind
        SP::InsertPlayer insertPlayer(*dbConn);
        insertPlayer.In_AccountDbId(2);
        insertPlayer.In_PlayerName(wstr.c_str(), wstr.size());
        insertPlayer.In_Level(1);
        insertPlayer.In_TotalExp(0);
        insertPlayer.In_MaxHp(200);
        insertPlayer.In_Hp(200);
        insertPlayer.In_Damage(20);
        insertPlayer.In_LocationX(0);
        insertPlayer.In_LocationY(0);
        insertPlayer.In_LocationZ(120.0f);
        insertPlayer.Execute();
    }

    {
        wstring wstr(L"kim1");

        // DBConnection UBind
        SP::InsertPlayer insertPlayer(*dbConn);
        insertPlayer.In_AccountDbId(3);
        insertPlayer.In_PlayerName(wstr.c_str(), wstr.size());
        insertPlayer.In_Level(1);
        insertPlayer.In_TotalExp(0);
        insertPlayer.In_MaxHp(200);
        insertPlayer.In_Hp(200);
        insertPlayer.In_Damage(20);
        insertPlayer.In_LocationX(0);
        insertPlayer.In_LocationY(0);
        insertPlayer.In_LocationZ(120.0f);
        insertPlayer.Execute();
    }
    */

    // 아이템DB
    /*
    {
        SP::InsertItem insertItem(*dbConn);
        insertItem.In_TemplateId(1);
        insertItem.In_Count(1);
        insertItem.In_Slot(1);
        insertItem.In_Equipped(false);
        insertItem.In_PlayerDbId(1);
        insertItem.Execute();
    }
    {
        SP::InsertItem insertItem(*dbConn);
        insertItem.In_TemplateId(2);
        insertItem.In_Count(1);
        insertItem.In_Slot(1);
        insertItem.In_Equipped(false);
        insertItem.In_PlayerDbId(2);
        insertItem.Execute();
    }
    {
        SP::InsertItem insertItem(*dbConn);
        insertItem.In_TemplateId(1);
        insertItem.In_Count(1);
        insertItem.In_Slot(1);
        insertItem.In_Equipped(false);
        insertItem.In_PlayerDbId(3);
        insertItem.Execute();
    }
    */

    GDBConnectionPool->Push(dbConn);
}

int main() {
    // 엔진 전역 객체, 데이터매니저
    CoreGlobal::Instance()->Init();
    DataManager::Instance()->Init();

    // DB 커넥션(thread count + 1), DB트랜잭션
    ASSERT_CRASH(GDBConnectionPool->Connect(4, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Database=MyDB;Trusted_Connection=Yes;"));
    DBTransaction::Instance()->Init();

    // 확률 시드
    srand(GetTickCount());

     
    DBConnection* dbConn = GDBConnectionPool->Pop();
    DBSynchronizer dbSync(*dbConn);
    
    // Parsing Xml
    if (dbSync.Synchronize(L"..\\Common\\Procedures\\GameDB.xml") == false) {
        cout << "XML Parsing Error" << endl;
        return 0;
    }
    else cout << "XML Parsing OK" << endl;
    
    // DB 사전 데이터 설정
    // DBDataSetOnce();

    GDBConnectionPool->Push(dbConn);

    // 룸매니저 게임룸 생성
    RoomManager::Instance()->Add(1);

    // 패킷 핸들러, 네트워크 시동
    ClientPacketHandler::Init();
    NetService* service = new NetService(make_shared<ClientSession>);
    service->Init();

    // IOCP, JOB 스레드 - 2개
    // 네트워크 FLUSH 스레드 1개
    GThreadManager->Launch(
        [&service]() {
            IocpAndJobTask(service);
        }
    );
    GThreadManager->Launch(
        [&service]() {
            IocpAndJobTask(service);
        }
    );
    GThreadManager->Launch(
        [&]() {
            NetworkFlushTask();
        }
    );

    GThreadManager->Join();
}


