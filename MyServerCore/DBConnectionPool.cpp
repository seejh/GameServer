#include "pch.h"
#include "DBConnectionPool.h"


#include"DBSynchronizer.h"


DBConnectionPool::DBConnectionPool() {}

DBConnectionPool::~DBConnectionPool()
{
	Clear();
}
// 서버 가동 후 풀에 있는 커넥션들 다 DB에 커넥 해놓음
bool DBConnectionPool::Connect(int32 connectionCount, const WCHAR* connectionString)
{
	lock_guard<mutex> _lock(_mutex);

	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_environment) != SQL_SUCCESS)
		return false;

	// sql odbc3버전
	if (::SQLSetEnvAttr(_environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
		return false;

	// 커넥션 카운트를 스레드 개수 만큼
	for (int32 i = 0; i < connectionCount; i++) {
		DBConnection* connection = xnew<DBConnection>();
		if (connection->Connect(_environment, connectionString) == false)
			return false;

		// 개인적 추가
		DBSynchronizer dbSync(*connection);

		_connections.push_back(connection);
	}

	return true;
}

void DBConnectionPool::Clear()
{
	lock_guard<mutex> _lock(_mutex);

	if (_environment != SQL_NULL_HANDLE) {
		::SQLFreeHandle(SQL_HANDLE_ENV, _environment);
		_environment = SQL_NULL_HANDLE;
	}

	for (DBConnection* connection : _connections)
		xdelete(connection);

	_connections.clear();
}
// 커넥션 여유분이 없으면 nullptr, 있으면 넘김
DBConnection* DBConnectionPool::Pop()
{
	lock_guard<mutex> _lock(_mutex);

	if (_connections.empty())
		return nullptr;

	DBConnection* connection = _connections.back();
	_connections.pop_back();

	return connection;
}
// 커넥션 반납
void DBConnectionPool::Push(DBConnection* connection)
{
	lock_guard<mutex> _lock(_mutex);

	_connections.push_back(connection);
}


