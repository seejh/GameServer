#pragma once


#include"DBDataModel.h"

// 잡큐로 변경 (일단 static, extern)


class ClientSession;
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

class SessionManager : public JobQueue {
public:
	static shared_ptr<SessionManager> Instance() {
		static shared_ptr<SessionManager> instance;
		if (instance == nullptr)
			instance = make_shared<SessionManager>();

		return instance;
	}

	void Init();

	void AddSession(shared_ptr<ClientSession> session);
	void RemoveSession(shared_ptr<ClientSession> session);
	int32 GetSessionCount();

	void FlushSend();

public:
	set<shared_ptr<ClientSession>> _sessions;
};
