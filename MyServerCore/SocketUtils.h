#pragma once

class SocketUtils
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
	static bool BindAnyAddress(SOCKET socket, uint16 port);
	static bool Listen(SOCKET socket);
	static bool HandleError(const char* cause);

	template<typename T>
	static void SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal);

	static bool SetUpdateAcceptSocket(SOCKET clientSocket, SOCKET listenSocket);
	static bool SetTcpNoDelay(SOCKET socket, bool flag);
	static bool SetReuseAddr(SOCKET socket, bool flag);
private:
	static bool WsaIoctl(SOCKET socket, GUID guid, LPVOID* lpfn);
};

template<typename T>
inline void SocketUtils::SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}
