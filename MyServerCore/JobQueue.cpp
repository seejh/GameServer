#include "pch.h"
#include "JobQueue.h"

#include"GlobalQueue.h"

// ������ push�� �� �ڱⰡ ó���̸� ������ ó���� ����ϴµ� ���� �׳� push�� �ϰ� �Ѿ
// �� Push �� �۷ι� ť�� ����(��ť) ���
void JobQueue::Push(shared_ptr<Job> job, bool pushOnly)
{
	// �� push
	_jobCount.fetch_add(1);
	_jobs.Push(job); // LockQueue�� ����ȭ

	// �۷ι� ť�� ���� ���
	GGlobalQueue->Push(shared_from_this());
}

// ��ť���� �� ������ ����
void JobQueue::Execute()
{
	while (true) {
		// �� ����
		vector<shared_ptr<Job>> jobs;
		_jobs.PopAll(OUT jobs); // LockQueue�� ����ȭ

		// ���� �� ����
		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++) 
			jobs[i]->Execute();

		// ���� ��� ó���ϴ� ���� ���� ���� ������ �Ŀ� ���� �⵵ ó��
		if (_jobCount.fetch_sub(jobCount) == jobCount) 
			return;
	}
}
