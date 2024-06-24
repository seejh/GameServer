#pragma once

#include"Job.h"
#include"LockQueue.h"

#include"JobTimer.h"

/*-------------------------------------------------------------------------------------
	JobQueue
	기본적으로 DoAsync(비동기 : 호출과 실행이 따로) 방식으로 동작한다.
--------------------------------------------------------------------------------------*/
class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	/*---------------------------------------------------------------------
		잡큐 Push
		본인(잡큐)에게 push 후 글로벌 큐에 등록
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
		잡타이머큐 Push
		잡타이머에 push(reserve)
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

	// 잡 push 후 글로벌 큐에 본인 등록
	void Push(shared_ptr<Job> job, bool pushOnly = false);
	
	// pop 후 잡 실행
	void Execute();


protected:
	LockQueue<shared_ptr<Job>> _jobs;
	atomic<int> _jobCount = 0;
};

