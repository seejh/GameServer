#pragma once

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(function<void(void)> callback);
	void Join();

	static void InitTLS();

private:
	mutex _mutex;
	vector<thread> _threads;
};

