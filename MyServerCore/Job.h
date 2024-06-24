#pragma once

class Job
{
public:
	Job(function<void()>&& callback) : _callback(std::move(callback)), _cancel(false) {}

	template<typename T, typename Ret, typename... Args>
	Job(std::shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args) : _cancel(false) {
		_callback = [owner, memFunc, args...]() {
			(owner.get()->*memFunc)(args...);
		};
	}

	void Execute() {
		if (_cancel == false)
			_callback();
	}

public:
	bool _cancel;
private:
	function<void()> _callback;
};

