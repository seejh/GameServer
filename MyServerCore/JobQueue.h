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

		현재 시스템은 잡큐를 고유하게 사용하게 만들어져있다.
		= 같은 일을 하는 잡큐를 여러개 만들 수 없는 구조

		잡큐에 일감을 등록하는 것은 내부적으로 락큐의 동기화로 등록,
		쌓인 일감을 처리하는 것은 글로벌 큐에서 하나의 스레드만 접근 가능하게 구현해놓고
		하나의 스레드에서도 쌓인 일감을 스택영역에(큐를 임시로 만듬) 복사하여 내부 락큐에
		대한 동기화 처리를 최소화하는 방식으로 사용

	---------------------------------------------------------------------*/
	void DoAsync(function<void()>&& callback) {
		/*
			함수를 잡의 형태로 찍어내고
			shared_ptr<Job> job = ObjectPool<Job>::MakeShared(std::move(callback));

			// 본인(잡큐) 내부의 큐에 잡을 순서대로 적재 후, 본인 자체를 글로벌 큐에 등록
			Push(job);

		*/
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

