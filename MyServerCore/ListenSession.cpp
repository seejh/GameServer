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
    if (!SocketUtils::SetReuseAddr(_listenSocket, true))
        return SocketUtils::HandleError("ListenSesison::Init SetReuseAddr()");

    // SocketOption TCPNODELAY
    if (!SocketUtils::SetTcpNoDelay(_listenSocket, true))
        return SocketUtils::HandleError("ListenSession::Init SetTcpNoDelay()");

    // ip, port
    string ip = "127.0.0.1";
    int port = 7777;
    SocketUtils::SetSockAddrIn(_serverAddr, ip, port);
    
    // BInd & Listen
    if (!SocketUtils::Bind(_listenSocket, _serverAddr))
        return SocketUtils::HandleError("ListenSession::Init Bind");
    if (!SocketUtils::Listen(_listenSocket))
        return SocketUtils::HandleError("ListenSession::Init Listen");

    // AcceptEvent
    _ownerNetService->_iocpCore->Register(shared_from_this());
    _acceptEvents = new AcceptEvent[_acceptCount];

    for (int i = 0; i < _acceptCount; i++) {
        _acceptEvents[i]._ownerIocpObject = shared_from_this();

        if (!RegisterAccept(&_acceptEvents[i]))
            return false;
    }


    cout << "ListenSession IP:" << ip << " Port:" << port << " Init OK" << endl;


    return true;
}

bool ListenSession::RegisterAccept(AcceptEvent* acceptEvent)
{
    DWORD dwBytes;

    // CreateSession
    shared_ptr<Session> session = _ownerNetService->_sessionFactory();

    session->_socket = SocketUtils::CreateSocket();

    if (session->_socket == INVALID_SOCKET)
        return false;

    // shared_ptr<Session> session = CreateSession();
    acceptEvent->_connectedSession = session;
    

    if (SocketUtils::_acceptEx(_listenSocket, session->_socket, session->_recvBuffer.BufferWritePos(), 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &dwBytes, reinterpret_cast<LPOVERLAPPED>(acceptEvent))) {

        if (WSAGetLastError() != WSA_IO_PENDING) {
            return false;
        }
    }

    return true;
}

void ListenSession::ProcessAccept(AcceptEvent* acceptEvent)
{   
    // Register session to IOCP & NetService
    shared_ptr<Session> session = acceptEvent->_connectedSession;
    session->_ownerNetService = _ownerNetService;
    _ownerNetService->_iocpCore->Register(session);
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
