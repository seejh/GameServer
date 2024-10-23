#pragma once

#include"DBDataModel.h"

/*
	�ϴ� Ŭ�󿡼��� ����Ʈ�� templateid �������� ����, ������ templateId �������� ����

	�߰�, ���� = DB �۾� ����
	������Ʈ = �ϷḦ ������ ������Ʈ(Update)�� �޸𸮿����� �۾�, 
	�Ϸ���º���(Complete)�� DB �۾�
*/

class Player;
class Quest;
class QuestManager {
public:
	// �߰�, ����
	bool Add(shared_ptr<Quest> quest);
	bool Remove(int questId);

	// ������Ʈ - �������� ����Ʈ ���� üũ
	void Update(QuestType type, int objectiveId, int quantity);

	// ������Ʈ - DB�۾� �� ���� �޸� ���� 
	bool Update(QuestDB questDB);
	void Complete(int questId);

public:
	weak_ptr<Player> _ownerPlayer;
	map<int, shared_ptr<Quest>> _quests;
};