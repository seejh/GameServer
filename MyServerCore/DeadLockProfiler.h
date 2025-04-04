#pragma once


/*------------------------
	DeadLockProfiler
	���� ��� Ǯ���� �� �������ϸ� �ϱ� ����
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
	// ��尡 �߰ߵ� ������ ����ϴ� �迭
	vector<int32>						_discoveredOrder;

	// ��尡 �߰ߵ� ����
	int32								_discoveredCount = 0;

	// Dfs(i)�� ���� �Ǿ����� ����
	vector<bool>						_finished;
	vector<int32>						_parent;
};

