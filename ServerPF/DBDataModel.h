#pragma once

#include<sqltypes.h>


enum class DB_STATE {
	NONE,
	UPDATE_NEED, UPDATE_SUCCESS, UPDATE_FAILED,
	DELETE_NEED, DELETE_SUCCESS, DELETE_FAILED,
	SELECT_NEED, SELECT_SUCCESS, SELECT_FAILED,
	INSERT_NEED, INSERT_SUCCESS, INSERT_FAILED,
};

/*=======================================================================
	SharedDB
=======================================================================*/
class ServerInfoDB {
public:
	int ServerDbId;
	WCHAR Name[50];
	WCHAR IpAddress[50];
	int Port;
	int BusyScore;
};

class TokenDB {
public:
	int TokenDbId;
	int AccountDbId;
	int Token;
	TIMESTAMP_STRUCT Expired;
};


/*=======================================================================
	GameDB
=======================================================================*/

class DB {
public:
	DB_STATE dbState;
};

/*------------------------------------------------------
	TABLE Player
-------------------------------------------------------*/
class PlayerDB {
public:
	int PlayerDbId;
	WCHAR PlayerName[50];
	int Level;
	int Exp;
	int MaxHp;
	int Hp;
	int Damage;
	float LocationX;
	float LocationY;
	float LocationZ;
	
	// m:1
	int AccountDbId;

	//AccountDB Account;

	// 1:m
	//vector<shared_ptr<ItemDB>> _items;

};

/*------------------------------------------------------
	TABLE Item
-------------------------------------------------------*/
enum class ItemDBTask {
	EQUIP, USE,
};

class ItemDB {
public:
	int ItemDbId;
	int TemplateId;
	int Count;
	int Slot;
	bool Equipped;

	// ForeignKey "Owner"
	int PlayerDbId;
	shared_ptr<PlayerDB> playerDb;

	//
	ItemDBTask taskType;
	DB_STATE dbState;
};

/*------------------------------------------------------
	TABLE Item
-------------------------------------------------------*/
class QuestDB {
public:
	int QuestDbId;
	int TemplateId;
	int Progress;
	bool Completed;
	int PlayerDbId;


	DB_STATE dbState;
};

/*------------------------------------------------------
	TABLE Account
-------------------------------------------------------*/
class AccountDB : public DB {
public:
	// PK �⺻Ű
	int AccountDbId;
	int AccountPw;
	WCHAR AccountName[50];

	// 1:m
	vector<PlayerDB> playerDBs;
};