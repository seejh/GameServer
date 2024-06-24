#include "pch.h"
#include "NetUtils.h"

LPFN_ACCEPTEX NetUtils::_acceptEx = nullptr;
LPFN_DISCONNECTEX NetUtils::_disconnectEx = nullptr;
LPFN_CONNECTEX NetUtils::_connectEx = nullptr;

bool NetUtils::Init() {
	if (!WsaStartup())  return HandleError("NetUtils::Init WsaStartup");

	SOCKET tmpSocket = CreateSocket();
	if (!WsaIoctl(tmpSocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&_acceptEx))) return HandleError("NetUtils::Init WsaIoctl AcceptEx");
	if (!WsaIoctl(tmpSocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&_disconnectEx))) return HandleError("NetUtils::Init WsaIoctl DisconnectEx");
	if (!WsaIoctl(tmpSocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&_connectEx))) return HandleError("NetUtils::Init WsaIoctl ConnectEx");
	CloseSocket(tmpSocket);

	return true;
}

bool NetUtils::WsaStartup() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	return true;
}

SOCKET NetUtils::CreateSocket() {
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
}

void NetUtils::CloseSocket(SOCKET socket) {
	::closesocket(socket);
}

void NetUtils::Cleanup() {
	::WSACleanup();
}

void NetUtils::SetSockAddrIn(OUT SOCKADDR_IN& sockAddr, string ip, int port) {
	::memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, ip.c_str(), &sockAddr.sin_addr);
	sockAddr.sin_port = ::htons(port);
}

bool NetUtils::Bind(SOCKET socket, SOCKADDR_IN& sockAddr) {
	return SOCKET_ERROR != ::bind(socket, (const sockaddr*)&sockAddr, sizeof(sockAddr));
}

bool NetUtils::Listen(SOCKET socket) {
	return SOCKET_ERROR != ::listen(socket, SOMAXCONN);
}

bool NetUtils::HandleError(const char* cause) {
	cout << cause << " Error : " << WSAGetLastError() << endl;
	return false;
}

void NetUtils::SetSockOpt() {

}

bool NetUtils::WsaIoctl(SOCKET socket, GUID guid, LPVOID* lpfn) {
	DWORD dwBytes = 0;
	return SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), lpfn, sizeof(*lpfn), &dwBytes, 0, 0);
}