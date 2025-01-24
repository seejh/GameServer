#pragma once

#include"DBDataModel.h"

/*
	일단 클라에서는 퀘스트를 templateid 기준으로 관리, 서버도 templateId 기준으로 관리

	추가, 삭제 = DB 작업 병행
	업데이트 = 완료를 제외한 업데이트(Update)는 메모리에서만 작업, 
	완료상태변경(Complete)은 DB 작업
*/

class Player;
class Quest;
class QuestManager {
public:
	// 추가, 삭제
	bool Add(shared_ptr<Quest> quest);
	bool Remove(int questId);

public:
	weak_ptr<Player> _ownerPlayer;
	map<int, shared_ptr<Quest>> _quests;
};