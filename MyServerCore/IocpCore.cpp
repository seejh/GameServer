#include "pch.h"
#include "IocpCore.h"

#include"IocpEvent.h"

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
}

IocpCore::~IocpCore()
{

}

void IocpCore::Register(shared_ptr<IocpObject> iocpObject)
{
	::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 0, 0);
}


void IocpCore::Dispatch(uint32 timeoutMs)

{
	DWORD dwRecvBytes;
	ULONG_PTR ulongPtr;
	IocpEvent* iocpEvent;

	bool result = ::GetQueuedCompletionStatus(_iocpHandle, &dwRecvBytes, &ulongPtr, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs);
	if (!result) {
		int32 errCode = ::WSAGetLastError();
		if (errCode == WAIT_TIMEOUT) 
			return;
		
		cout << "[IocpCore] Dispatch() Error, RecvLen: " << dwRecvBytes << ", Result: " << std::boolalpha << result << ", ErrorCode: " << errCode << endl;
	}
	
	shared_ptr<IocpObject> iocpObject = iocpEvent->_ownerIocpObject;
	iocpObject->Dispatch(iocpEvent, dwRecvBytes);
	
}
