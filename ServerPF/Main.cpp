// MyServerPF.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include"pch.h"
#include"NetService.h"

#include"GameSession.h"
#include"DBBind.h"
#include"DBConnectionPool.h"
#include"GameClientPacketHandler.h"

#include"GameSessionManager.h"
#include"GameRoom.h"
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
#include"RedisManager.h"

#include"ConfigManager.h"

#pragma region ThreadTask

void GameServerIocpAndJob(NetService* service) {
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

        GameSessionManager::Instance()->FlushSend();

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
    // 엔진 전역 객체
    CoreGlobal::Instance()->Init();
    
    // 컨피그, 데이터 매니저
    ASSERT_CRASH(DataManager::Instance()->Init());
    ASSERT_CRASH(ConfigManager::Instance()->LoadConfigFile("..\\Common\\Data\\ServerConfig.json"));

    // 컨피그 로드
    ServerConfig gameServerCfg;
    ASSERT_CRASH(ConfigManager::Instance()->LoadConfigByName("GameServer", &gameServerCfg));

    RDBConfig gameDBCfg;
    ASSERT_CRASH(ConfigManager::Instance()->LoadConfigByName("GameDB", &gameDBCfg));
    
    RDBConfig sharedDBCfg;
    ASSERT_CRASH(ConfigManager::Instance()->LoadConfigByName("SharedDB", &sharedDBCfg));
   
    MDBConfig redisCfg;
    ASSERT_CRASH(ConfigManager::Instance()->LoadConfigByName("Redis", &redisCfg));

    // DB
    ASSERT_CRASH(DBManager::Instance()->InitA(
        gameDBCfg._connectionCount, gameDBCfg._connectionString.c_str(),
        sharedDBCfg._connectionCount, sharedDBCfg._connectionString.c_str(),
        redisCfg._connectionCount, redisCfg._ip.c_str(), redisCfg._port
    ));

    // 확률 시드
    srand(GetTickCount());
    
    // DB싱크
    // DBSync();

    // 룸매니저 게임룸 생성, 
    GameSessionManager::Instance()->Init();
    RoomManager::Instance()->Init();
    DBManager::Instance()->_sharedDbManager->DoAsync(&SharedDBManager::UpdateServerInfo);
    
    // 게임 - 패킷 핸들러, 네트워크 서비스
    GameClientPacketHandler::Init();
    ServerService* gameServerService = new ServerService(make_shared<GameSession>,
        gameServerCfg._ip,
        gameServerCfg._port,
        gameServerCfg._threadCount
    );
    ASSERT_CRASH(gameServerService->Init());

    // 게임 서버 - 스레드
    for (int i = 0; i < gameServerCfg._threadCount; i++) {
        GThreadManager->Launch(
            [&gameServerService]() {GameServerIocpAndJob(gameServerService); }
        );
    }

    GThreadManager->Join();
}
