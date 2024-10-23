#include "pch.h"
#include "Quest.h"

#include"DataManager.h"

shared_ptr<Quest> Quest::MakeQuest(QuestDB questDB)
{
	// 생성
	shared_ptr<Quest> quest = make_shared<Quest>();

	// 조회
	auto it = DataManager::Instance()->_questTable.find(questDB.TemplateId);
	if (it == DataManager::Instance()->_questTable.end()) {
		cout << "Quest::MakeQuest(QuestDB) Error - Can't Find QuestData QuestTemplateId:" << questDB.TemplateId << endl;
		return nullptr;
	}
		
	// 데이터 설정
	quest->_questData = it->second;
	quest->_questInfo.set_questdbid(questDB.QuestDbId);
	quest->_questInfo.set_templateid(questDB.TemplateId);
	quest->_questInfo.set_playerdbid(questDB.PlayerDbId);
	quest->_questInfo.set_progress(questDB.Progress);
	quest->_questInfo.set_completed(questDB.Completed);

	// 리턴
	return quest;
}

