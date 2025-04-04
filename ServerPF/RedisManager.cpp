#include "pch.h"
#include "RedisManager.h"


/*-------------------------------------------------------------------------------
	RedisConnection
-------------------------------------------------------------------------------*/
bool RedisConnection::Connect(const char* ip, int32 port)
{
	_conn = redisConnectWithTimeout(ip, port, timeout);
	if (_conn == nullptr || _conn->err) {
		// cout << "" << endl;

		redisFree(_conn);

		return false;
	}

	return true;
}

bool RedisConnection::SetKey(const char* key, const char* value)
{
	bool result = false;

	_reply = reinterpret_cast<redisReply*>(redisCommand(_conn, "SET %s %s", key, value));
	if (_reply != nullptr && _reply->type == REDIS_REPLY_STATUS)
		result = true;

	freeReplyObject(_reply);

	return result;
}

bool RedisConnection::GetKey(const char* key, OUT string& outValue)
{
	bool result = false;

	_reply = reinterpret_cast<redisReply*>(redisCommand(_conn, "GET %s", key));
	if (_reply != nullptr && _reply->type == REDIS_REPLY_STRING) {
		outValue = _reply->str;
		result = true;
	}

	freeReplyObject(_reply);

	return result;
}

bool RedisConnection::Increase(const char* key, OUT int32& afterValue)
{
	bool result = false;

	_reply = reinterpret_cast<redisReply*>(redisCommand(_conn, "INCR %s", key));
	if (_reply != nullptr && _reply->type == REDIS_REPLY_INTEGER) {
		afterValue = _reply->integer;
		result = true;
	}

	freeReplyObject(_reply);

	return result;
}

bool RedisConnection::Decrease(const char* key, OUT int32& afterValue)
{
	bool result = false;

	_reply = reinterpret_cast<redisReply*>(redisCommand(_conn, "DECR %s", key));
	if (_reply != nullptr && _reply->type == REDIS_REPLY_INTEGER) {
		afterValue = _reply->integer;
		result = true;
	}

	freeReplyObject(_reply);

	return result;
}

/*-------------------------------------------------------------------------------
	RedisConnectionPool
-------------------------------------------------------------------------------*/
bool RedisConnectionPool::Connect(const char* ip, int32 port, int32 connectionCounts)
{
	RedisConnection* redisConn;
	for (int i = 0; i < connectionCounts; i++) {
		redisConn = new RedisConnection();
		if (redisConn->Connect(ip, port) == false)
			return false;

		_connectionPool.push(redisConn);
	}

	return true;
}

RedisConnection* RedisConnectionPool::Pop()
{
	lock_guard<mutex> lock(_mutex);

	RedisConnection* conn = nullptr;

	if (_connectionPool.empty() == false) {

		conn = _connectionPool.front();
		_connectionPool.pop();
	}

	return conn;
}

void RedisConnectionPool::Push(RedisConnection* conn)
{
	lock_guard<mutex> lock(_mutex);

	if(conn != nullptr)
		_connectionPool.push(conn);
}


/*-------------------------------------------------------------------------------
	RedisManager
-------------------------------------------------------------------------------*/
bool RedisDBManager::Connect(const char* ip, int32 port, int32 connectionCounts)
{
	if (_pool == nullptr)
		_pool = new RedisConnectionPool();

	if (_pool->Connect(ip, port, connectionCounts) == false) {
		cout << "[RedisManager] Error : Connect FAilED" << endl;
		return false;
	}
		
	cout << "[RedisManager] Connect OK" << endl;

	return true;
}

RedisConnection* RedisDBManager::Pop()
{
	return _pool->Pop();
}

void RedisDBManager::Push(RedisConnection* conn)
{
	_pool->Push(conn);
}


