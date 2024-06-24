
#pragma once
#include "Types.h"
#include <windows.h>
#include "DBBind.h"

namespace SP
{
	
    class InsertAccount : public DBBind<2,0>
    {
    public:
    	InsertAccount(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertAccount(?,?)}") { }
    	void In_AccountPw(int32& v) { BindParam(0, v); };
    	void In_AccountPw(int32&& v) { _accountPw = std::move(v); BindParam(0, _accountPw); };
    	template<int32 N> void In_AccountName(WCHAR(&v)[N]) { BindParam(1, v); };
    	template<int32 N> void In_AccountName(const WCHAR(&v)[N]) { BindParam(1, v); };
    	void In_AccountName(WCHAR* v, int32 count) { BindParam(1, v, count); };
    	void In_AccountName(const WCHAR* v, int32 count) { BindParam(1, v, count); };

    private:
    	int32 _accountPw = {};
    };

    class SelectAccountByAccountDbId : public DBBind<1,3>
    {
    public:
    	SelectAccountByAccountDbId(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectAccountByAccountDbId(?)}") { }
    	void In_AccountDbId(int32& v) { BindParam(0, v); };
    	void In_AccountDbId(int32&& v) { _accountDbId = std::move(v); BindParam(0, _accountDbId); };
    	void Out_AccountDbId(OUT int32& v) { BindCol(0, v); };
    	void Out_AccountPw(OUT int32& v) { BindCol(1, v); };
    	template<int32 N> void Out_AccountName(OUT WCHAR(&v)[N]) { BindCol(2, v); };

    private:
    	int32 _accountDbId = {};
    };

    class SelectAccountByAccountName : public DBBind<1,3>
    {
    public:
    	SelectAccountByAccountName(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectAccountByAccountName(?)}") { }
    	template<int32 N> void In_AccountName(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_AccountName(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_AccountName(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_AccountName(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void Out_AccountDbId(OUT int32& v) { BindCol(0, v); };
    	void Out_AccountPw(OUT int32& v) { BindCol(1, v); };
    	template<int32 N> void Out_AccountName(OUT WCHAR(&v)[N]) { BindCol(2, v); };

    private:
    };

    class InsertPlayer : public DBBind<10,0>
    {
    public:
    	InsertPlayer(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertPlayer(?,?,?,?,?,?,?,?,?,?)}") { }
    	template<int32 N> void In_PlayerName(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_PlayerName(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_PlayerName(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_PlayerName(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_Level(int32& v) { BindParam(1, v); };
    	void In_Level(int32&& v) { _level = std::move(v); BindParam(1, _level); };
    	void In_TotalExp(int32& v) { BindParam(2, v); };
    	void In_TotalExp(int32&& v) { _totalExp = std::move(v); BindParam(2, _totalExp); };
    	void In_MaxHp(int32& v) { BindParam(3, v); };
    	void In_MaxHp(int32&& v) { _maxHp = std::move(v); BindParam(3, _maxHp); };
    	void In_Hp(int32& v) { BindParam(4, v); };
    	void In_Hp(int32&& v) { _hp = std::move(v); BindParam(4, _hp); };
    	void In_Damage(int32& v) { BindParam(5, v); };
    	void In_Damage(int32&& v) { _damage = std::move(v); BindParam(5, _damage); };
    	void In_LocationX(float& v) { BindParam(6, v); };
    	void In_LocationX(float&& v) { _locationX = std::move(v); BindParam(6, _locationX); };
    	void In_LocationY(float& v) { BindParam(7, v); };
    	void In_LocationY(float&& v) { _locationY = std::move(v); BindParam(7, _locationY); };
    	void In_LocationZ(float& v) { BindParam(8, v); };
    	void In_LocationZ(float&& v) { _locationZ = std::move(v); BindParam(8, _locationZ); };
    	void In_AccountDbId(int32& v) { BindParam(9, v); };
    	void In_AccountDbId(int32&& v) { _accountDbId = std::move(v); BindParam(9, _accountDbId); };

    private:
    	int32 _level = {};
    	int32 _totalExp = {};
    	int32 _maxHp = {};
    	int32 _hp = {};
    	int32 _damage = {};
    	float _locationX = {};
    	float _locationY = {};
    	float _locationZ = {};
    	int32 _accountDbId = {};
    };

    class UpdatePlayer : public DBBind<9,0>
    {
    public:
    	UpdatePlayer(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spUpdatePlayer(?,?,?,?,?,?,?,?,?)}") { }
    	void In_PlayerDbId(int32& v) { BindParam(0, v); };
    	void In_PlayerDbId(int32&& v) { _playerDbId = std::move(v); BindParam(0, _playerDbId); };
    	void In_Level(int32& v) { BindParam(1, v); };
    	void In_Level(int32&& v) { _level = std::move(v); BindParam(1, _level); };
    	void In_TotalExp(int32& v) { BindParam(2, v); };
    	void In_TotalExp(int32&& v) { _totalExp = std::move(v); BindParam(2, _totalExp); };
    	void In_MaxHp(int32& v) { BindParam(3, v); };
    	void In_MaxHp(int32&& v) { _maxHp = std::move(v); BindParam(3, _maxHp); };
    	void In_Hp(int32& v) { BindParam(4, v); };
    	void In_Hp(int32&& v) { _hp = std::move(v); BindParam(4, _hp); };
    	void In_Damage(int32& v) { BindParam(5, v); };
    	void In_Damage(int32&& v) { _damage = std::move(v); BindParam(5, _damage); };
    	void In_LocationX(float& v) { BindParam(6, v); };
    	void In_LocationX(float&& v) { _locationX = std::move(v); BindParam(6, _locationX); };
    	void In_LocationY(float& v) { BindParam(7, v); };
    	void In_LocationY(float&& v) { _locationY = std::move(v); BindParam(7, _locationY); };
    	void In_LocationZ(float& v) { BindParam(8, v); };
    	void In_LocationZ(float&& v) { _locationZ = std::move(v); BindParam(8, _locationZ); };

    private:
    	int32 _playerDbId = {};
    	int32 _level = {};
    	int32 _totalExp = {};
    	int32 _maxHp = {};
    	int32 _hp = {};
    	int32 _damage = {};
    	float _locationX = {};
    	float _locationY = {};
    	float _locationZ = {};
    };

    class SelectPlayerName : public DBBind<1,1>
    {
    public:
    	SelectPlayerName(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectPlayerName(?)}") { }
    	template<int32 N> void In_PlayerName(WCHAR(&v)[N]) { BindParam(0, v); };
    	template<int32 N> void In_PlayerName(const WCHAR(&v)[N]) { BindParam(0, v); };
    	void In_PlayerName(WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void In_PlayerName(const WCHAR* v, int32 count) { BindParam(0, v, count); };
    	void Out_PlayerDbId(OUT int32& v) { BindCol(0, v); };

    private:
    };

    class SelectPlayerByPlayerDbId : public DBBind<1,11>
    {
    public:
    	SelectPlayerByPlayerDbId(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectPlayerByPlayerDbId(?)}") { }
    	void In_PlayerDbId(int32& v) { BindParam(0, v); };
    	void In_PlayerDbId(int32&& v) { _playerDbId = std::move(v); BindParam(0, _playerDbId); };
    	void Out_PlayerDbId(OUT int32& v) { BindCol(0, v); };
    	template<int32 N> void Out_PlayerName(OUT WCHAR(&v)[N]) { BindCol(1, v); };
    	void Out_Level(OUT int32& v) { BindCol(2, v); };
    	void Out_TotalExp(OUT int32& v) { BindCol(3, v); };
    	void Out_MaxHp(OUT int32& v) { BindCol(4, v); };
    	void Out_Hp(OUT int32& v) { BindCol(5, v); };
    	void Out_Damage(OUT int32& v) { BindCol(6, v); };
    	void Out_LocationX(OUT float& v) { BindCol(7, v); };
    	void Out_LocationY(OUT float& v) { BindCol(8, v); };
    	void Out_LocationZ(OUT float& v) { BindCol(9, v); };
    	void Out_AccountDbId(OUT int32& v) { BindCol(10, v); };

    private:
    	int32 _playerDbId = {};
    };

    class SelectPlayerByAccountDbId : public DBBind<1,11>
    {
    public:
    	SelectPlayerByAccountDbId(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectPlayerByAccountDbId(?)}") { }
    	void In_AccountDbId(int32& v) { BindParam(0, v); };
    	void In_AccountDbId(int32&& v) { _accountDbId = std::move(v); BindParam(0, _accountDbId); };
    	void Out_PlayerDbId(OUT int32& v) { BindCol(0, v); };
    	template<int32 N> void Out_PlayerName(OUT WCHAR(&v)[N]) { BindCol(1, v); };
    	void Out_Level(OUT int32& v) { BindCol(2, v); };
    	void Out_TotalExp(OUT int32& v) { BindCol(3, v); };
    	void Out_MaxHp(OUT int32& v) { BindCol(4, v); };
    	void Out_Hp(OUT int32& v) { BindCol(5, v); };
    	void Out_Damage(OUT int32& v) { BindCol(6, v); };
    	void Out_LocationX(OUT float& v) { BindCol(7, v); };
    	void Out_LocationY(OUT float& v) { BindCol(8, v); };
    	void Out_LocationZ(OUT float& v) { BindCol(9, v); };
    	void Out_AccountDbId(OUT int32& v) { BindCol(10, v); };

    private:
    	int32 _accountDbId = {};
    };

    class SelectTest : public DBBind<0,1>
    {
    public:
    	SelectTest(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectTest}") { }
    	void Out_Identity(OUT int64& v) { BindCol(0, v); };

    private:
    };

    class InsertItem : public DBBind<5,0>
    {
    public:
    	InsertItem(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spInsertItem(?,?,?,?,?)}") { }
    	void In_TemplateId(int32& v) { BindParam(0, v); };
    	void In_TemplateId(int32&& v) { _templateId = std::move(v); BindParam(0, _templateId); };
    	void In_Count(int32& v) { BindParam(1, v); };
    	void In_Count(int32&& v) { _count = std::move(v); BindParam(1, _count); };
    	void In_Slot(int32& v) { BindParam(2, v); };
    	void In_Slot(int32&& v) { _slot = std::move(v); BindParam(2, _slot); };
    	void In_Equipped(bool& v) { BindParam(3, v); };
    	void In_Equipped(bool&& v) { _equipped = std::move(v); BindParam(3, _equipped); };
    	void In_PlayerDbId(int32& v) { BindParam(4, v); };
    	void In_PlayerDbId(int32&& v) { _playerDbId = std::move(v); BindParam(4, _playerDbId); };

    private:
    	int32 _templateId = {};
    	int32 _count = {};
    	int32 _slot = {};
    	bool _equipped = {};
    	int32 _playerDbId = {};
    };

    class UpdateItem : public DBBind<6,0>
    {
    public:
    	UpdateItem(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spUpdateItem(?,?,?,?,?,?)}") { }
    	void In_ItemDbId(int32& v) { BindParam(0, v); };
    	void In_ItemDbId(int32&& v) { _itemDbId = std::move(v); BindParam(0, _itemDbId); };
    	void In_TemplateId(int32& v) { BindParam(1, v); };
    	void In_TemplateId(int32&& v) { _templateId = std::move(v); BindParam(1, _templateId); };
    	void In_Count(int32& v) { BindParam(2, v); };
    	void In_Count(int32&& v) { _count = std::move(v); BindParam(2, _count); };
    	void In_Slot(int32& v) { BindParam(3, v); };
    	void In_Slot(int32&& v) { _slot = std::move(v); BindParam(3, _slot); };
    	void In_Equipped(bool& v) { BindParam(4, v); };
    	void In_Equipped(bool&& v) { _equipped = std::move(v); BindParam(4, _equipped); };
    	void In_PlayerDbId(int32& v) { BindParam(5, v); };
    	void In_PlayerDbId(int32&& v) { _playerDbId = std::move(v); BindParam(5, _playerDbId); };

    private:
    	int32 _itemDbId = {};
    	int32 _templateId = {};
    	int32 _count = {};
    	int32 _slot = {};
    	bool _equipped = {};
    	int32 _playerDbId = {};
    };

    class DeleteItem : public DBBind<1,0>
    {
    public:
    	DeleteItem(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spDeleteItem(?)}") { }
    	void In_ItemDbId(int32& v) { BindParam(0, v); };
    	void In_ItemDbId(int32&& v) { _itemDbId = std::move(v); BindParam(0, _itemDbId); };

    private:
    	int32 _itemDbId = {};
    };

    class SelectIdentity : public DBBind<0,1>
    {
    public:
    	SelectIdentity(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectIdentity}") { }
    	void Out_Identity(OUT int64& v) { BindCol(0, v); };

    private:
    };

    class SelectItemByItemDbId : public DBBind<1,6>
    {
    public:
    	SelectItemByItemDbId(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectItemByItemDbId(?)}") { }
    	void In_ItemDbId(int32& v) { BindParam(0, v); };
    	void In_ItemDbId(int32&& v) { _itemDbId = std::move(v); BindParam(0, _itemDbId); };
    	void Out_ItemDbId(OUT int32& v) { BindCol(0, v); };
    	void Out_TemplateId(OUT int32& v) { BindCol(1, v); };
    	void Out_Count(OUT int32& v) { BindCol(2, v); };
    	void Out_Slot(OUT int32& v) { BindCol(3, v); };
    	void Out_Equipped(OUT bool& v) { BindCol(4, v); };
    	void Out_PlayerDbId(OUT int32& v) { BindCol(5, v); };

    private:
    	int32 _itemDbId = {};
    };

    class SelectItemByPlayerDbId : public DBBind<1,6>
    {
    public:
    	SelectItemByPlayerDbId(DBConnection& conn) : DBBind(conn, L"{CALL dbo.spSelectItemByPlayerDbId(?)}") { }
    	void In_PlayerDbId(int32& v) { BindParam(0, v); };
    	void In_PlayerDbId(int32&& v) { _playerDbId = std::move(v); BindParam(0, _playerDbId); };
    	void Out_ItemDbId(OUT int32& v) { BindCol(0, v); };
    	void Out_TemplateId(OUT int32& v) { BindCol(1, v); };
    	void Out_Count(OUT int32& v) { BindCol(2, v); };
    	void Out_Slot(OUT int32& v) { BindCol(3, v); };
    	void Out_Equipped(OUT bool& v) { BindCol(4, v); };
    	void Out_PlayerDbId(OUT int32& v) { BindCol(5, v); };

    private:
    	int32 _playerDbId = {};
    };


     
};