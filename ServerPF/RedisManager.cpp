#include "pch.h"
#include "RedisManager.h"

bool RedisManager::Connect(const char* ip, int port)
{
	struct timeval timeout = { 1, 500'000 }; // 1.5 seconds
	_context = redisConnectWithTimeout(ip, port, timeout);
	if (_context == nullptr) {
		cout << "Redis Connection Error : nullptr" << endl;
		return false;
	}

	if (_context->err) {
		cout << "Redis Connection Error : " << _context->errstr << endl;
		redisFree(_context);
		return false;
	}

	return true;
}

bool RedisManager::StringGet(const char* key, string& getValue)
{
	_reply = reinterpret_cast<redisReply*>(redisCommand(_context, "GET ", key));
	if (_reply->type != REDIS_REPLY_STRING) {
		cout << "Redis Get Error : Can't Find Key" << endl;
		return false;
	}

	getValue = _reply->str;

	freeReplyObject(_reply);

	return true;
}

void RedisManager::StringSet(const char* key, const char* value)
{
	_reply = reinterpret_cast<redisReply*>(redisCommand(_context, "SET ", key, value));

	// TODO : Log

	freeReplyObject(_reply);
}
