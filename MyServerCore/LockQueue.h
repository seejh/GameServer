#pragma once

template<typename T>
class LockQueue
{
public:
	void Push(T item) {
		lock_guard<mutex> lock(_mutex);
		_items.push(item);
	}

	T Pop() {
		lock_guard<mutex> lock(_mutex);
		if (_items.empty()) // ex) return shared_ptr<Session> == nullptr
			return T();

		T ret = _items.front();
		_items.pop();

		return ret;
	}
	
	void PopAll(OUT vector<T>& items) {
		lock_guard<mutex> lock(_mutex);
		
		while (!_items.empty()) {
			T item = _items.front();
			_items.pop();

			items.push_back(item);
		}
	}

	void Clear() {
		lock_guard<mutex> lock(_mutex);
		_items = queue<T>();
	}
private:
	mutex _mutex;
	queue<T> _items;
};

