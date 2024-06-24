#pragma once

#include"DataContents.h"
#include"rapidjson/document.h"

class DataManager
{
public:
	static DataManager* Instance() {
		static DataManager instance;
		return &instance;
	}

	bool Init();

	// TEST
	bool OpenAndParseJson(string path, rapidjson::Document& doc);

	// Load
	bool LoadStatTable();
	bool LoadItemTable();
	bool LoadMonsterTable();
	bool LoadSkillTable();
	
private:
	DataManager(){}
	DataManager(const DataManager&);
	DataManager& operator=(const DataManager&);
public:
	map<int, MonsterData> _monsterTable;
	map<int, ItemData*> _itemTable;
	map<int, StatData> _statTable;
	map<int, SkillData> _skillTable;
};

