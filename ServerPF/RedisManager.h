#pragma once

#include<hiredis-master/hiredis.h>


/*
	Redis 싱글 스레드 동작(이벤트 루프 사용, 비동기 입출력)
*/

/*-------------------------------------------------------------------------------
	RedisConnection
-------------------------------------------------------------------------------*/
class RedisConnection {
	struct timeval timeout = { 1,50'0000 }; // 1.5 sec
public:
	bool Connect(const char* ip, int32 port);

	bool SetKey(const char* key, const char* value);
	bool GetKey(const char* key, OUT string& outValue);
	bool Increase(const char* key, OUT int32& afterValue);
	bool Decrease(const char* key, OUT int32& afterValue);
	redisContext* ToContext() {
		return _conn;
	}
public:
	redisContext* _conn = nullptr;
	redisReply* _reply = nullptr;
};

/*-------------------------------------------------------------------------------
	RedisConnectionPool
-------------------------------------------------------------------------------*/
class RedisConnectionPool {
public:
	bool Connect(const char* ip, int32 port, int32 connectionCounts = 1);
	
	RedisConnection* Pop();
	void Push(RedisConnection* conn);
public:
	mutex _mutex;
	queue<RedisConnection*> _connectionPool;
};

/*-------------------------------------------------------------------------------
	RedisManager
-------------------------------------------------------------------------------*/
class RedisDBManager {
public:

	bool Connect(const char* ip, int32 port, int32 connectionCounts);
	
	RedisConnection* Pop();
	void Push(RedisConnection* conn);

public:
	RedisConnectionPool* _pool;
};
