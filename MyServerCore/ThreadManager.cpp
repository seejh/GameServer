#include "pch.h"
#include "ThreadManager.h"

#include"CoreGlobal.h"

ThreadManager::ThreadManager()
{}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	lock_guard<mutex> lock(_mutex);

	_threads.push_back(thread([=]() {
		InitTLS();
		callback();
		}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
		if (t.joinable())
			t.join();

	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static atomic<uint32> threadIdGenerator = 1;
	TLSThreadId = threadIdGenerator.fetch_add(1);
}
