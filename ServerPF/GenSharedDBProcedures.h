
#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

namespace SP
{
	
    class InsertServerInfo : public DBBind<4,0>
    {
    public:
    	InsertServerInfo(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertServerInfo(?,?,?,?)}") { }
    	template<int32 N> void In_Name(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_Name(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_Name(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_Name(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	template<int32 N> void In_IpAddress(WCHAR(&v)[N]) { BindParam(1, v); };
    	template<int32 N> void In_IpAddress(const WCHAR(&v)[N]) { BindParam(1, v); };
    	void In_IpAddress(WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void In_IpAddress(const WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void In_Port(int32& v) { BindParam(2, v); };
    	void In_Port(int32&& v) { _port = std::move(v); BindParam(2, _port); };
    	void In_BusyScore(int32& v) { BindParam(3, v); };
    	void In_BusyScore(int32&& v) { _busyScore = std::move(v); BindParam(3, _busyScore); };

    private:
    	int32 _port = {};
    	int32 _busyScore = {};
    };

    class UpdateServerInfo : public DBBind<5,0>
    {
    public:
    	UpdateServerInfo(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spUpdateServerInfo(?,?,?,?,?)}") { }
    	void In_ServerDbId(int32& v) { BindParam(0, v); };
    	void In_ServerDbId(int32&& v) { _serverDbId = std::move(v); BindParam(0, _serverDbId); };
    	template<int32 N> void In_Name(WCHAR(&v)[N]) { BindParam(1, v); };
    	template<int32 N> void In_Name(const WCHAR(&v)[N]) { BindParam(1, v); };
    	void In_Name(WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void In_Name(const WCHAR* v, int32 count) { BindParam(1, v, count); };
    	template<int32 N> void In_IpAddress(WCHAR(&v)[N]) { BindParam(2, v); };
    	template<int32 N> void In_IpAddress(const WCHAR(&v)[N]) { BindParam(2, v); };
    	void In_IpAddress(WCHAR* v, int32 count) { BindParam(2, v, count); };
    	void In_IpAddress(const WCHAR* v, int32 count) { BindParam(2, v, count); };
    	void In_Port(int32& v) { BindParam(3, v); };
    	void In_Port(int32&& v) { _port = std::move(v); BindParam(3, _port); };
    	void In_BusyScore(int32& v) { BindParam(4, v); };
    	void In_BusyScore(int32&& v) { _busyScore = std::move(v); BindParam(4, _busyScore); };

    private:
    	int32 _serverDbId = {};
    	int32 _port = {};
    	int32 _busyScore = {};
    };

    class SelectServerInfo : public DBBind<1,5>
    {
    public:
    	SelectServerInfo(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectServerInfo(?)}") { }
    	void In_ServerDbId(int32& v) { BindParam(0, v); };
    	void In_ServerDbId(int32&& v) { _serverDbId = std::move(v); BindParam(0, _serverDbId); };
    	void Out_ServerDbId(OUT int32& v) { BindCol(0, v); };
    	template<int32 N> void Out_Name(OUT WCHAR(&v)[N]) { BindCol(1, v); };
    	template<int32 N> void Out_IpAddress(OUT WCHAR(&v)[N]) { BindCol(2, v); };
    	void Out_Port(OUT int32& v) { BindCol(3, v); };
    	void Out_BusyScore(OUT int32& v) { BindCol(4, v); };

    private:
    	int32 _serverDbId = {};
    };

    class InsertToken : public DBBind<3,0>
    {
    public:
    	InsertToken(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertToken(?,?,?)}") { }
    	void In_AccountDbId(int32& v) { BindParam(0, v); };
    	void In_AccountDbId(int32&& v) { _accountDbId = std::move(v); BindParam(0, _accountDbId); };
    	void In_Token(int32& v) { BindParam(1, v); };
    	void In_Token(int32&& v) { _token = std::move(v); BindParam(1, _token); };
    	void In_Expired(TIMESTAMP_STRUCT& v) { BindParam(2, v); };
    	void In_Expired(TIMESTAMP_STRUCT&& v) { _expired = std::move(v); BindParam(2, _expired); };

    private:
    	int32 _accountDbId = {};
    	int32 _token = {};
    	TIMESTAMP_STRUCT _expired = {};
    };

    class SelectToken : public DBBind<1,4>
    {
    public:
    	SelectToken(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectToken(?)}") { }
    	void In_TokenDbId(int32& v) { BindParam(0, v); };
    	void In_TokenDbId(int32&& v) { _tokenDbId = std::move(v); BindParam(0, _tokenDbId); };
    	void Out_TokenDbId(OUT int32& v) { BindCol(0, v); };
    	void Out_AccountDbId(OUT int32& v) { BindCol(1, v); };
    	void Out_Token(OUT int32& v) { BindCol(2, v); };
    	void Out_Expired(OUT TIMESTAMP_STRUCT& v) { BindCol(3, v); };

    private:
    	int32 _tokenDbId = {};
    };


     
};