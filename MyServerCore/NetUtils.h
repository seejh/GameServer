#pragma once

class NetUtils
{
public:
	static LPFN_ACCEPTEX _acceptEx;
	static LPFN_DISCONNECTEX _disconnectEx;
	static LPFN_CONNECTEX _connectEx;

	static bool Init();
	static bool WsaStartup();
	static SOCKET CreateSocket();
	static void CloseSocket(SOCKET socket) ;
	static void Cleanup();
	static void SetSockAddrIn(OUT SOCKADDR_IN& sockAddr, string ip, int port);
	static bool Bind(SOCKET socket, SOCKADDR_IN& sockAddr);
	static bool Listen(SOCKET socket);
	static bool HandleError(const char* cause);

	static void SetSockOpt();
private:
	static bool WsaIoctl(SOCKET socket, GUID guid, LPVOID* lpfn);
};

