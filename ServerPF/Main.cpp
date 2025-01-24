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

#include<bitset>

#include"GenProcedures.h"
#include"GenSharedDBProcedures.h"
#include"DBSynchronizer.h"
#include"DBDataModel.h"

#include"DBManager.h"
#include"RoomManager.h"
#include"GameDBManager.h"
#include"SharedDBManager.h"
#include"ConfigManager.h"
#include"RedisManager.h"

#include<hiredis-master/hiredis.h>

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

void DBSync() {
    // GameDB Sync
    DBConnection* gameDbConn = DBManager::Instance()->_gameDbManager->_dbConn;
    DBSynchronizer gameDBSync(*gameDbConn);
    ASSERT_CRASH(gameDBSync.Synchronize(L"..\\Common\\Procedures\\GameDB.xml"));
    
    // SharedDB Sync
    DBConnection* sharedDBConn = DBManager::Instance()->_sharedDbManager->_dbConn;
    DBSynchronizer sharedDBSync(*sharedDBConn);
    ASSERT_CRASH(sharedDBSync.Synchronize(L"..\\Common\\Procedures\\SharedDB.xml"));
}

int main() {
    uint32 threadCounts = thread::hardware_concurrency();

    // 엔진 전역 객체, 데이터매니저
    CoreGlobal::Instance()->Init();
    ASSERT_CRASH(DataManager::Instance()->Init());
    ASSERT_CRASH(ConfigManager::Instance()->Init());
    ASSERT_CRASH(DBManager::Instance()->Init(
        threadCounts, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Database=MyDB;Trusted_Connection=Yes;",
        threadCounts, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Database=SharedDB;Trusted_Connection=Yes;",
        threadCounts, "127.0.0.1", 6379));

    // 확률 시드
    srand(GetTickCount());
    
    // DB싱크
    // DBSync();

    // 룸매니저 게임룸 생성, 
    SessionManager::Instance()->Init();
    RoomManager::Instance()->Init();
    DBManager::Instance()->_sharedDbManager->DoAsync(&SharedDBManager::UpdateServerInfo);

    // 패킷 핸들러, 네트워크 시동
    ClientPacketHandler::Init();
    ServerService* service = new ServerService(make_shared<ClientSession>,
        ConfigManager::Instance()->_config.IpAddress,
        ConfigManager::Instance()->_config.Port,
        1);
    if (service->Init() == false)
        return 0;

    // 
    for (int i = 0; i < threadCounts; i++) {
        GThreadManager->Launch(
            [&service]() {IocpAndJobTask(service); }
        );
    }

    GThreadManager->Join();
}
