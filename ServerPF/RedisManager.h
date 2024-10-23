#pragma once

#include<hiredis-master/hiredis.h>

class RedisManager : public JobQueue {
public:
	// 싱글톤
	static shared_ptr<RedisManager> Instance() {
		static shared_ptr<RedisManager> instance;
		if (instance == nullptr)
			instance = make_shared<RedisManager>();

		return instance;
	}

	// 커넥트 외에 전부다 async로 사용해야 함.
	// 커넥트
	bool Connect(const char* ip, int port);
	
	// Get & Set
	bool StringGet(const char* key, string& value);
	void StringSet(const char* key, const char* value);

	// 


public:
	redisContext* _context;
	redisReply* _reply;
};