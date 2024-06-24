#include "pch.h"
#include "CoreGlobal.h"

#include"Memory.h"
#include"DBConnectionPool.h"

#include"GlobalQueue.h"
#include"JobTimer.h"
#include"ThreadManager.h"
#include"ConsoleLog.h"


/*--------------------------------------------------------------------
	Global Instance
--------------------------------------------------------------------*/

Memory*						GMemory = nullptr;
ThreadManager*				GThreadManager = nullptr;
GlobalQueue*				GGlobalQueue = nullptr;
JobTimer*					GJobTimer = nullptr;
DBConnectionPool*			GDBConnectionPool = nullptr;
ConsoleLog*					GConsoleLogger = nullptr;

/*--------------------------------------------------------------
	CoreGlobal
---------------------------------------------------------------*/

CoreGlobal::CoreGlobal(){}

void CoreGlobal::Init()
{
	cout << "CoreGlobal Init" << endl;

	GMemory					= new Memory();
	GThreadManager			= new ThreadManager();
	GGlobalQueue			= new GlobalQueue();
	GJobTimer				= new JobTimer();
	GDBConnectionPool		= new DBConnectionPool();
	GConsoleLogger			= new ConsoleLog();
}



/*--------------------------------------------------------------
	TLS (Thread Local Storage)
---------------------------------------------------------------*/
thread_local uint32 TLSThreadId = 0;


