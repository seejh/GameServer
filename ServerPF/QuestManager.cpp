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
	// �̹� �����ϸ� ����
	auto it = _quests.find(quest->_questInfo.templateid());
	if (it != _quests.end()) {
		cout << "QuestManager::Add() Error - QuestTemplateId:" << quest->_questInfo.templateid() << " is Already Exists" << endl;
		return false;
	}

	// ����
	_quests[quest->_questInfo.templateid()] = quest;
	
	return true;
}

bool QuestManager::Remove(int questId)
{
	// �ش� ����Ʈ Ž��
	auto it = _quests.find(questId);
	if (it == _quests.end()) {
		// ���� ���� : ����
		cout << "QuestManager::Remove() Error - Can't Find QuestTemplateId:" << questId << endl;
		return false;
	}
	
	// ����
	if (_quests.erase(questId) == 0) {
		// ���� ���� : ���� ���� 0
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
//	// ��Ŷ
//	shared_ptr<Player> player = _ownerPlayer.lock();
//	
//	PROTOCOL::S_UpdateQuest toPkt;
//	toPkt.mutable_questinfo()->CopyFrom(it->second->_questInfo);
//	auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
//	player->_ownerSession->SendPacket(sendBuffer);
//
//	return true;
//}

