#pragma once
#include<sql.h>
#include<sqlext.h>

enum {
	WVARCHAR_MAX = 4000,
	BINARY_MAX = 8000
};

class DBConnection
{
public:
	// 실질적으로 DB에 커넥트 하는 함수
	bool Connect(SQLHENV henv, const WCHAR* connectionString);
	void Clear();

	// SQL query 실행
	bool Execute(const WCHAR* query);
	// 데이터가 있으면 받아오겠다.
	bool Fetch();
	// 데이터를 몇개나 받아왔는지
	int32 GetRowCount();
	// statement에 바인드된 인자 해제
	void Unbind();

	// TEST
	// 트랜잭션 시작 - 오토 커밋 비활성화
	void SetAutoCommit(bool onOff);
	bool Commit();
	bool Rollback();

public:

	// 원래 아래 두개로 했는데 받아올 때 마다 타입등을 적어주는게 힘들어서
	// SQL은 signed int밖에 없음, bool도 없음 걍 int8 사용
	bool BindParam(int32 paramIndex, bool* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, float* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, double* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, int8* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, int16* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, int32* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, int64* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool BindParam(int32 paramIndex, const WCHAR* str, SQLLEN* index);
	bool BindParam(int32 paramIndex, const BYTE* bin, int32 size, SQLLEN* index);

	bool BindCol(int32 columnIndex, bool* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, float* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, double* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, int8* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, int16* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, int32* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, int64* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, TIMESTAMP_STRUCT* value, SQLLEN* index);
	bool BindCol(int32 columnIndex, WCHAR* str, int32 size, SQLLEN* index);
	bool BindCol(int32 columnIndex, BYTE* bin, int32 size, SQLLEN* index);

private:
	// 쿼리를 실행할 때 보내줄 인자 바인드
	bool BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);

	// 쿼리를 실행할 때 받을 인자 바인드
	bool BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);

	void HandleError(SQLRETURN ret);

private:
	// 실제로 DB 커넥션을 담당하는 핸들
	SQLHDBC _connection = SQL_NULL_HANDLE;
	// SQL 쿼리 인자를 담는
	SQLHSTMT _statement = SQL_NULL_HANDLE;
};

