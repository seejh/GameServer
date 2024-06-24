#include "pch.h"
#include "JobQueue.h"

#include"GlobalQueue.h"

// 원본은 push할 때 자기가 처음이면 본인이 처리를 담당하는데 나는 그냥 push만 하고 넘어감
// 잡 Push 후 글로벌 큐에 본인(잡큐) 등록
void JobQueue::Push(shared_ptr<Job> job, bool pushOnly)
{
	// 잡 push
	_jobCount.fetch_add(1);
	_jobs.Push(job); // LockQueue로 동기화

	// 글로벌 큐에 본인 등록
	GGlobalQueue->Push(shared_from_this());
}

// 잡큐에서 잡 빼내서 실행
void JobQueue::Execute()
{
	while (true) {
		// 잡 빼냄
		vector<shared_ptr<Job>> jobs;
		_jobs.PopAll(OUT jobs); // LockQueue로 동기화

		// 빼낸 잡 실행
		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++) 
			jobs[i]->Execute();

		// 뺴낸 잡들 처리하는 동안 들어온 잡이 있으면 후에 들어온 잡도 처리
		if (_jobCount.fetch_sub(jobCount) == jobCount) 
			return;
	}
}
