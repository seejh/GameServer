#pragma once

#include"DataContents.h"

class DataManager
{
public:
	static DataManager* Instance() {
		static DataManager instance;
		return &instance;
	}

	// 로드 전부
	bool Init();

	// 로드 테이블
	bool LoadStatTable();
	bool LoadItemTable();
	bool LoadMonsterTable();
	bool LoadSkillTable();
	bool LoadNpcTable();
	bool LoadQuestTable();

private:
	DataManager(){}
	DataManager(const DataManager&);
	DataManager& operator=(const DataManager&);
public:
	map<int, MonsterData*> _monsterTable;
	map<int, ItemData*> _itemTable;
	map<int, StatData> _statTable;
	map<int, SkillData> _skillTable;
	map<int, NpcData*> _npcTable;
	map<int, QuestData*> _questTable;
};

