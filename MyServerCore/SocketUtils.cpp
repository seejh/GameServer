#include "pch.h"
#include "SocketUtils.h"

LPFN_ACCEPTEX SocketUtils::_acceptEx = nullptr;
LPFN_DISCONNECTEX SocketUtils::_disconnectEx = nullptr;
LPFN_CONNECTEX SocketUtils::_connectEx = nullptr;

bool SocketUtils::Init() {
	if (!WsaStartup()) 
		return HandleError("NetUtils::Init WsaStartup");

	SOCKET tmpSocket = CreateSocket();

	if (!WsaIoctl(tmpSocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&_acceptEx))) 
		return HandleError("NetUtils::Init WsaIoctl AcceptEx");
	if (!WsaIoctl(tmpSocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&_disconnectEx))) 
		return HandleError("NetUtils::Init WsaIoctl DisconnectEx");
	if (!WsaIoctl(tmpSocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&_connectEx))) 
		return HandleError("NetUtils::Init WsaIoctl ConnectEx");
	
	CloseSocket(tmpSocket);

	return true;
}

bool SocketUtils::WsaStartup() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	return true;
}

SOCKET SocketUtils::CreateSocket() {
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
}

void SocketUtils::CloseSocket(SOCKET socket) {
	::closesocket(socket);
}

void SocketUtils::Cleanup() {
	::WSACleanup();
}

void SocketUtils::SetSockAddrIn(OUT SOCKADDR_IN& sockAddr, string ip, int port) {
	::memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, ip.c_str(), &sockAddr.sin_addr);
	sockAddr.sin_port = ::htons(port);
}

bool SocketUtils::Bind(SOCKET socket, SOCKADDR_IN& sockAddr) {
	return SOCKET_ERROR != ::bind(socket, (const sockaddr*)&sockAddr, sizeof(sockAddr));
}

bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)
{
	SOCKADDR_IN myAddress;

	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketUtils::Listen(SOCKET socket) {
	return SOCKET_ERROR != ::listen(socket, SOMAXCONN);
}

bool SocketUtils::HandleError(const char* cause) {
	cout << cause << " Error : " << WSAGetLastError() << endl;
	return false;
}

bool SocketUtils::SetUpdateAcceptSocket(SOCKET clientSocket, SOCKET listenSocket) {
	return SOCKET_ERROR != setsockopt(clientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&listenSocket), sizeof(SOCKET));
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag) {
	return SOCKET_ERROR != setsockopt(socket, SOL_SOCKET, TCP_NODELAY, reinterpret_cast<char*>(&flag), sizeof(bool));
}

bool SocketUtils::SetReuseAddr(SOCKET socket, bool flag) {
	return SOCKET_ERROR != setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&flag), sizeof(bool));
}

bool SocketUtils::WsaIoctl(SOCKET socket, GUID guid, LPVOID* lpfn) {
	DWORD dwBytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), lpfn, sizeof(*lpfn), &dwBytes, 0, 0);
}