#include "pch.h"
#include "QuestManager.h"
#include"DataManager.h"
#include"DataContents.h"
#include"Quest.h"
#include"DBManager.h"
#include"GameDBManager.h"
#include"GenProcedures.h"
#include"GameObject.h"
#include"ClientPacketHandler.h"
#include"ObjectManager.h"
#include"Room.h"


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

void QuestManager::Update(QuestType type, int objectiveId, int quantity)
{
	// �ϴ� �޸𸮿����� ������Ʈ
	for (auto p : _quests) {
		if (p.second->_questInfo.completed() == true) 
			continue;
			
		if (p.second->_questData->type != type)
			continue;

		if (p.second->_questData->objectiveId != objectiveId)
			continue;
			
		if (p.second->_questInfo.progress() >= p.second->_questData->quantity)
			continue;
			
		// ������Ʈ
		p.second->_questInfo.set_progress(p.second->_questInfo.progress() + quantity);

		// ��Ŷ
		PROTOCOL::S_UpdateQuest toPkt;
		toPkt.mutable_questinfo()->CopyFrom(p.second->_questInfo);
		
		auto sendBuffer = ClientPacketHandler::MakeSendBuffer(toPkt);
		shared_ptr<Player> player = _ownerPlayer.lock();
		if (player == nullptr)
			return;

		player->_ownerSession->SendPacket(sendBuffer);
	}
}

bool QuestManager::Update(QuestDB questDB)
{
	auto it = _quests.find(questDB.TemplateId);
	if (it == _quests.end()) {
		cout << "QuestManager::Update(QuestDB) Error - Can't Find QuestId:" << questDB.TemplateId << endl;
		return false;
	}

	it->second->_questInfo.set_questdbid(questDB.QuestDbId);
	it->second->_questInfo.set_templateid(questDB.TemplateId);
	it->second->_questInfo.set_playerdbid(questDB.PlayerDbId);
	it->second->_questInfo.set_progress(questDB.Progress);
	it->second->_questInfo.set_completed(questDB.Completed);
	
	return true;
}

void QuestManager::Complete(int questId)
{
	
}
