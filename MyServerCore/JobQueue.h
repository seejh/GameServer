#pragma once

#include"Job.h"
#include"LockQueue.h"

#include"JobTimer.h"

/*-------------------------------------------------------------------------------------
	JobQueue
	�⺻������ DoAsync(�񵿱� : ȣ��� ������ ����) ������� �����Ѵ�.
--------------------------------------------------------------------------------------*/
class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	/*---------------------------------------------------------------------
		��ť Push
		����(��ť)���� push �� �۷ι� ť�� ���
	---------------------------------------------------------------------*/
	void DoAsync(function<void()>&& callback) {
		Push(ObjectPool<Job>::MakeShared(std::move(callback)));
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::* memFunc)(Args...), Args... args) {
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, forward<Args>(args)...));
	}
	/*---------------------------------------------------------------------
		��Ÿ�̸�ť Push
		��Ÿ�̸ӿ� push(reserve)
	---------------------------------------------------------------------*/
	shared_ptr<Job> DoTimer(uint64 tickAfter, function<void()>&& callback) {
		shared_ptr<Job> job = ObjectPool<Job>::MakeShared(move(callback));
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);

		return job;
	}

	template<typename T, typename Ret, typename... Args>
	shared_ptr<Job> DoTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args) {
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		shared_ptr<Job> job = ObjectPool<Job>::MakeShared(owner, memFunc, forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);

		return job;
	}

public:
	void ClearJobs() { _jobs.Clear(); }

	// �� push �� �۷ι� ť�� ���� ���
	void Push(shared_ptr<Job> job, bool pushOnly = false);
	
	// pop �� �� ����
	void Execute();


protected:
	LockQueue<shared_ptr<Job>> _jobs;
	atomic<int> _jobCount = 0;
};

