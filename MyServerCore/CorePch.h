#pragma once


#include<iostream>
#define WIN32_LEAN_AND_MEAN

#include<WinSock2.h>
#include<MSWSock.h>
#include<WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include<Windows.h>

#include<thread>
#include<mutex>
#include<atomic>
#include<chrono>

#include<functional>
#include<string>

#include<map>
#include<set>
#include<vector>
#include<queue>
#include<stack>
#include<concurrent_queue.h>

#include<conio.h>

using namespace std;

#include"Types.h"
#include"CoreGlobal.h"

#include"SocketUtils.h"
#include"Memory.h"
#include"ObjectPool.h"
#include"LockQueue.h"
#include"JobQueue.h"
#include"ConsoleLog.h"


