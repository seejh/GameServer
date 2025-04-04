#pragma once


/*------------------------
	DeadLockProfiler
	락을 잡고 풀어줄 때 프로파일링 하기 위해
------------------------*/

class DeadLockProfiler
{
public:
	void	PushLock(const char* name);
	void	PopLock(const char* name);
	void	CheckCycle();

private:
	void	Dfs(int32 index);

private:
	unordered_map<const char*, int32>	_nameToId;
	unordered_map<int32, const char*>	_idToName;

	map<int32, set<int32>>				_lockHistory;

	mutex _lock;

private:
	// 노드가 발견된 순서를 기록하는 배열
	vector<int32>						_discoveredOrder;

	// 노드가 발견된 순사
	int32								_discoveredCount = 0;

	// Dfs(i)가 종료 되었는지 여부
	vector<bool>						_finished;
	vector<int32>						_parent;
};

