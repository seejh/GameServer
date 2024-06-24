#pragma once


class IocpEvent;

class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual void Dispatch(IocpEvent* iocpEvent, int len) abstract;
	virtual HANDLE GetHandle() abstract;
};

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	void Register(shared_ptr<IocpObject> iocpObject);

	void Dispatch(uint32 timeoutMs = INFINITE);

public:
	HANDLE _iocpHandle;
};

