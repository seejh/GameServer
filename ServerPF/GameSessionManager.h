#pragma once


#include"DBDataModel.h"

// 잡큐로 변경 (일단 static, extern)


class GameSession;
//class SessionManager
//{
//public:
//	static SessionManager* Instance() {
//		static SessionManager _instance;
//		return &_instance;
//
//	}
//
//	void AddSession(shared_ptr<ClientSession> session);
//	void RemoveSession(shared_ptr<ClientSession> session);
//	int32 GetSessionCount();
//
//	// TEST
//	void FlushSend();
//
//private:
//	SessionManager() {}
//	SessionManager(const SessionManager&);
//	SessionManager operator=(const SessionManager&);
//public:
//
//	mutex _mutex;
//	set<shared_ptr<ClientSession>> _sessions;
//};

class GameSessionManager : public JobQueue {
public:
	static shared_ptr<GameSessionManager> Instance() {
		static shared_ptr<GameSessionManager> instance;
		if (instance == nullptr)
			instance = make_shared<GameSessionManager>();

		return instance;
	}

	void Init();

	void AddSession(shared_ptr<GameSession> session);
	void RemoveSession(shared_ptr<GameSession> session);
	int32 GetSessionCount();

	void FlushSend();

public:
	set<shared_ptr<GameSession>> _sessions;
};
