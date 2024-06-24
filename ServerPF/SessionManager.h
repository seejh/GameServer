#pragma once

class ClientSession;
class SessionManager
{

public:
	static SessionManager* Instance() {
		static SessionManager _instance;
		return &_instance;

	}

	void AddSession(shared_ptr<ClientSession> session);
	void RemoveSession(shared_ptr<ClientSession> session);

	// TEST
	void FlushSend();

private:
	SessionManager() {}
	SessionManager(const SessionManager&);
	SessionManager operator=(const SessionManager&);
public:

	mutex _mutex;
	set<shared_ptr<ClientSession>> _sessions;
};

