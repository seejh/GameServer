#pragma once

/*-----------------------------------------------------------------------
	JobData
	잡과 잡의 소유주(잡큐)에 대한 정보를 가짐
-------------------------------------------------------------------------*/
class Job;
class JobQueue;
struct JobData {
	JobData(weak_ptr<JobQueue> owner, shared_ptr<Job> job) : _owner(owner), _job(job) {}

	// 잡과 소속 잡큐 참조
	weak_ptr<JobQueue> _owner;
	shared_ptr<Job> _job;
};

/*-----------------------------------------------------------------------
	TimerItem
	잡과 잡의 소유주, 실행 시간을 가짐
-------------------------------------------------------------------------*/
struct TimerItem {
	bool operator<(const TimerItem& other) const {
		return _executeTick > other._executeTick;
	}

	uint64 _executeTick = 0;
	JobData* _jobData = nullptr;
};


/*-----------------------------------------------------------------------
	JobTimer
	예약과 분배
	시간에 따라 필요한 잡을 따로 보관하고 잡의 실행시간이 되면 해당 잡의 소유 잡큐로
	잡을 Push 한다. 즉, 잡을 보관만하고 처리는 잡큐로 넣어서 잡큐가 처리하게 함.

	글로벌 객체로, 코어글로벌에 등록, 하나로만 관리되게 설계
	락 사용
	타이머잡의 시간 우선도를 비교해서 적재하기 위해 우선순위 큐 사용

	현 문제, 고려 사항
	greater로 처리하면 빌드가 안되는 문제가 생김
	원본은 WRITE_LOCK 사용, 여기선 mutex, 예외 사항있나 생각할 필요 있음
-------------------------------------------------------------------------*/
class JobTimer
{
public:
	// 예약 : 잡타이머에 잡을 push
	void Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, shared_ptr<Job> job);
	
	// 분배 : 잡타이머에서 잡 pop, 해당 잡 소유 잡큐로 push
	void Distribute(uint64 now);

	// 잡타이머 초기화
	void Clear();
private:
	mutex _m;
	priority_queue<TimerItem, vector<TimerItem, StlAllocator<TimerItem>>, less<TimerItem>> _items;
	atomic<bool> _distributing = false;
};

