#pragma once

#include"DBDataModel.h"

class Quest {
public:
	static shared_ptr<Quest> MakeQuest(QuestDB questDB);
	
public:
	PROTOCOL::QuestInfo _questInfo;
	QuestData* _questData;
};

