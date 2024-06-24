#include "pch.h"
#include "JobTimer.h"


/*-----------------------------------------------------------------------
	JobTimer
-------------------------------------------------------------------------*/
void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, shared_ptr<Job> job)
{
	// Ÿ�̸��� ����
	const uint64 executeTick = GetTickCount64() + tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

	// �� �ɰ�
	lock_guard<mutex> lock(_m);

	// ��Ÿ�̸�ť�� ����
	_items.push(TimerItem{ executeTick, jobData });
}

// ��Ÿ�̸ӿ��� �� pop, �ش� �� ���� ��ť�� push, �׷��� ���ֵ̹� �й�
void JobTimer::Distribute(uint64 now)
{
	// �ٸ� �����尡 �й����̸� �н�
	if (_distributing.exchange(true) == true)
		return;

	// ����ð��� �� ���� ������ ����
	vector<TimerItem, StlAllocator<TimerItem>> items;
	{
		// �� �ɰ�
		lock_guard<mutex> lock(_m);

		while (_items.empty() == false) {
			// ���� �ð� üũ
			const TimerItem& timerItem = _items.top();
			
			if (now < timerItem._executeTick)
				break;

			// ���� �ð��̸� ����
			items.push_back(timerItem);
			_items.pop();
		}
	}

	// !!! �߿�     !!!!!!!!!!
	// �ش� ���� ���� ��ť�� ���� Push(��ť�� ó���ϰ�)
	for (TimerItem& item : items) {
		if (shared_ptr<JobQueue> owner = item._jobData->_owner.lock()) 
			owner->Push(item._jobData->_job);

		ObjectPool<JobData>::Push(item._jobData);
	}

	// �й� ��
	_distributing.store(false);
}

void JobTimer::Clear()
{
	// �� �ɰ�
	lock_guard<mutex> lock(_m);

	// ���� ��ť���� ��� ����, �޸� �ݳ�
	while (_items.empty() == false) {
		const TimerItem& timerItem = _items.top();
		ObjectPool<JobData>::Push(timerItem._jobData);
		_items.pop();
	}
}
