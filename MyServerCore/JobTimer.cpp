#include "pch.h"
#include "JobTimer.h"


/*-----------------------------------------------------------------------
	JobTimer
-------------------------------------------------------------------------*/
void JobTimer::Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, shared_ptr<Job> job)
{
	// 타이머잡 생성
	const uint64 executeTick = GetTickCount64() + tickAfter;
	JobData* jobData = ObjectPool<JobData>::Pop(owner, job);

	// 락 걸고
	lock_guard<mutex> lock(_m);

	// 잡타이머큐에 적재
	_items.push(TimerItem{ executeTick, jobData });
}

// 잡타이머에서 잡 pop, 해당 잡 소유 잡큐로 push, 그래서 네이밍도 분배
void JobTimer::Distribute(uint64 now)
{
	// 다른 스레드가 분배중이면 패스
	if (_distributing.exchange(true) == true)
		return;

	// 실행시간이 된 잡이 있으면 빼냄
	vector<TimerItem, StlAllocator<TimerItem>> items;
	{
		// 락 걸고
		lock_guard<mutex> lock(_m);

		while (_items.empty() == false) {
			// 실행 시간 체크
			const TimerItem& timerItem = _items.top();
			
			if (now < timerItem._executeTick)
				break;

			// 실행 시간이면 빼냄
			items.push_back(timerItem);
			_items.pop();
		}
	}

	// !!! 중요     !!!!!!!!!!
	// 해당 잡의 소유 잡큐로 잡을 Push(잡큐가 처리하게)
	for (TimerItem& item : items) {
		if (shared_ptr<JobQueue> owner = item._jobData->_owner.lock()) 
			owner->Push(item._jobData->_job);

		ObjectPool<JobData>::Push(item._jobData);
	}

	// 분배 끝
	_distributing.store(false);
}

void JobTimer::Clear()
{
	// 락 걸고
	lock_guard<mutex> lock(_m);

	// 내부 잡큐에서 잡들 제거, 메모리 반납
	while (_items.empty() == false) {
		const TimerItem& timerItem = _items.top();
		ObjectPool<JobData>::Push(timerItem._jobData);
		_items.pop();
	}
}
