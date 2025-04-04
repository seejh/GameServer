#include "pch.h"
#include "QuestManager.h"
#include"DataManager.h"
#include"DataContents.h"
#include"Quest.h"
#include"DBManager.h"
#include"GameDBManager.h"
#include"GenProcedures.h"
#include"GameObject.h"
#include"GameClientPacketHandler.h"
#include"ObjectManager.h"
#include"GameRoom.h"


bool QuestManager::Add(shared_ptr<Quest> quest)
{
	// 이미 존재하면 실패
	auto it = _quests.find(quest->_questInfo.templateid());
	if (it != _quests.end()) {
		cout << "QuestManager::Add() Error - QuestTemplateId:" << quest->_questInfo.templateid() << " is Already Exists" << endl;
		return false;
	}

	// 삽입
	_quests[quest->_questInfo.templateid()] = quest;
	
	return true;
}

bool QuestManager::Remove(int questId)
{
	// 해당 퀘스트 탐색
	auto it = _quests.find(questId);
	if (it == _quests.end()) {
		// 삭제 실패 : 없음
		cout << "QuestManager::Remove() Error - Can't Find QuestTemplateId:" << questId << endl;
		return false;
	}
	
	// 삭제
	if (_quests.erase(questId) == 0) {
		// 삭제 실패 : 삭제 개수 0
		cout << "QuestManager::Remove() Error - Erase QuestTemplateId:" << questId << " Eraze Nothing" << endl;
		return false;
	}

	return true;
}

//bool QuestManager::Update(QuestDB questDB)
//{
//	auto it = _quests.find(questDB.TemplateId);
//	if (it == _quests.end()) {
//		cout << "QuestManager::Update(QuestDB) Error - Can't Find QuestId:" << questDB.TemplateId << endl;
//		return false;
//	}
//
//	it->second->_questInfo.set_questdbid(questDB.QuestDbId);
//	it->second->_questInfo.set_templateid(questDB.TemplateId);
//	it->second->_questInfo.set_playerdbid(questDB.PlayerDbId);
//	it->second->_questInfo.set_progress(questDB.Progress);
//	it->second->_questInfo.set_completed(questDB.Completed);
//	
//	// 패킷
//	shared_ptr<Player> player = _ownerPlayer.lock();
//	
//	PROTOCOL::S_UpdateQuest toPkt;
//	toPkt.mutable_questinfo()->CopyFrom(it->second->_questInfo);
//	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
//	player->_ownerSession->SendPacket(sendBuffer);
//
//	return true;
//}

