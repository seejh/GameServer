#pragma once

#include<hiredis-master/hiredis.h>

class RedisManager : public JobQueue {
public:
	// �̱���
	static shared_ptr<RedisManager> Instance() {
		static shared_ptr<RedisManager> instance;
		if (instance == nullptr)
			instance = make_shared<RedisManager>();

		return instance;
	}

	// Ŀ��Ʈ �ܿ� ���δ� async�� ����ؾ� ��.
	// Ŀ��Ʈ
	bool Connect(const char* ip, int port);
	
	// Get & Set
	bool StringGet(const char* key, string& value);
	void StringSet(const char* key, const char* value);

	// 


public:
	redisContext* _context;
	redisReply* _reply;
};