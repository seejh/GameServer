#include "pch.h"
#include "ListenSession.h"

#include"NetService.h"
#include"IocpCore.h"
#include"Session.h"
#include"IocpEvent.h"

ListenSession::ListenSession(NetService* netService) : _ownerNetService(netService)
{
}

ListenSession::~ListenSession()
{
}

HANDLE ListenSession::GetHandle()
{
    return reinterpret_cast<HANDLE>(_listenSocket);
}

void ListenSession::Dispatch(IocpEvent* iocpEvent, int len)
{
    AcceptEvent* acceptEvent = reinterpret_cast<AcceptEvent*>(iocpEvent);
    ProcessAccept(acceptEvent);
}

bool ListenSession::Init()
{
    if (!SocketUtils::Init())
        return SocketUtils::HandleError("ListenSession Init::NetUtils::Init");

    // Create ListenSocket
    _listenSocket = SocketUtils::CreateSocket();
    if (_listenSocket == INVALID_SOCKET)
        return SocketUtils::HandleError("ListenSession::Init INVLAID listenSocket");

    // SocketOption REUSE
    if (SocketUtils::SetReuseAddr(_listenSocket, true) == false)
        return SocketUtils::HandleError("ListenSesison::Init SetReuseAddr()");

    // SocketOption TCPNODELAY
    if (SocketUtils::SetTcpNoDelay(_listenSocket, true) == false)
        return SocketUtils::HandleError("ListenSession::Init SetTcpNoDelay()");
    
    // BInd & Listen
    if (SocketUtils::Bind(_listenSocket, _ownerNetService->_sockAddr) == false)
        return SocketUtils::HandleError("ListenSession::Init Bind");
    if (SocketUtils::Listen(_listenSocket) == false)
        return SocketUtils::HandleError("ListenSession::Init Listen");

    // AcceptEvent
    _ownerNetService->_iocpCore->Register(shared_from_this());
    
    for (int i = 0; i < _ownerNetService->_maxSessionCounts; i++) {
        AcceptEvent* acceptEvent = new AcceptEvent();
        acceptEvent->_ownerIocpObject = shared_from_this();
        _acceptEvents.push_back(acceptEvent);
        if (RegisterAccept(acceptEvent) == false)
            return false;
    }

    return true;
}

bool ListenSession::RegisterAccept(AcceptEvent* acceptEvent)
{
    DWORD dwBytes;

    shared_ptr<Session> session = _ownerNetService->CreateSession();
    if (session->_socket == INVALID_SOCKET)
        return false;

    acceptEvent->_connectedSession = session;
    
    if (SocketUtils::_acceptEx(_listenSocket, session->_socket, session->_recvBuffer.BufferWritePos(), 0, 
        sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, reinterpret_cast<LPOVERLAPPED>(acceptEvent))) {
        
        if (WSAGetLastError() != WSA_IO_PENDING) 
            return false;
    }

    return true;
}

void ListenSession::ProcessAccept(AcceptEvent* acceptEvent)
{   
    // Register session to IOCP & NetService
    shared_ptr<Session> session = acceptEvent->_connectedSession;
    _ownerNetService->AddSession(session);

    // Set Sock Opt
    SocketUtils::SetUpdateAcceptSocket(session->_socket, _listenSocket);

    // Session Register Recv
    session->OnConnected();
    session->_isConnected.store(true);
    session->RegisterRecv();

    // Reset AcceptEvent

    acceptEvent->OverlappedReset();
    acceptEvent->_connectedSession.reset();

    // Register Accept Again
    RegisterAccept(acceptEvent);
}

NetService* ListenSession::OwnerNetService()
{
    return nullptr;
}
