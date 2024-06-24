#pragma once

/*-----------------------------------------------------------------------
	JobData
	��� ���� ������(��ť)�� ���� ������ ����
-------------------------------------------------------------------------*/
class Job;
class JobQueue;
struct JobData {
	JobData(weak_ptr<JobQueue> owner, shared_ptr<Job> job) : _owner(owner), _job(job) {}

	// ��� �Ҽ� ��ť ����
	weak_ptr<JobQueue> _owner;
	shared_ptr<Job> _job;
};

/*-----------------------------------------------------------------------
	TimerItem
	��� ���� ������, ���� �ð��� ����
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
	����� �й�
	�ð��� ���� �ʿ��� ���� ���� �����ϰ� ���� ����ð��� �Ǹ� �ش� ���� ���� ��ť��
	���� Push �Ѵ�. ��, ���� �������ϰ� ó���� ��ť�� �־ ��ť�� ó���ϰ� ��.

	�۷ι� ��ü��, �ھ�۷ι��� ���, �ϳ��θ� �����ǰ� ����
	�� ���
	Ÿ�̸����� �ð� �켱���� ���ؼ� �����ϱ� ���� �켱���� ť ���

	�� ����, ��� ����
	greater�� ó���ϸ� ���尡 �ȵǴ� ������ ����
	������ WRITE_LOCK ���, ���⼱ mutex, ���� �����ֳ� ������ �ʿ� ����
-------------------------------------------------------------------------*/
class JobTimer
{
public:
	// ���� : ��Ÿ�̸ӿ� ���� push
	void Reserve(uint64 tickAfter, weak_ptr<JobQueue> owner, shared_ptr<Job> job);
	
	// �й� : ��Ÿ�̸ӿ��� �� pop, �ش� �� ���� ��ť�� push
	void Distribute(uint64 now);

	// ��Ÿ�̸� �ʱ�ȭ
	void Clear();
private:
	mutex _m;
	priority_queue<TimerItem, vector<TimerItem, StlAllocator<TimerItem>>, less<TimerItem>> _items;
	atomic<bool> _distributing = false;
};

