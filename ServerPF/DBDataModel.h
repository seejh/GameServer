#pragma once

class PlayerDB;
class ItemDB;

/*------------------------------------------------------
	TABLE Account
-------------------------------------------------------*/
class AccountDB {
public:
	// PK 기본키
	int AccountDbId;
	int AccountPw;
	WCHAR AccountName[50];
	
	// 1:m
	//vector<PlayerDB> _players;
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
};



/*
class AccountDB {
public:
public:
	// PK 기본키
	int AccountDbId;
	int AccountPw;
	WCHAR AccountName[50];

	// 1:m
	vector<PlayerDB> _players;
};

struct PlayerDB {
	int PlayerDbId;
	WCHAR PlayerName[50];

	// m:1
	int AccountDbId;
	AccountDB Account;

	// 1:m
	vector<shared_ptr<ItemDB>> _items;

	// stat
	int Level;
	int TotalExp;
	int MaxHp;
	int Hp;
	int Damage;
};

struct ItemDB {
	int ItemDbId;
	int TemplateId;
	int Count;
	int Slot;
	bool Equipped;

	// ForeignKey "Owner"
	int PlayerDbId;
	shared_ptr<PlayerDB> playerDb;
};
*/