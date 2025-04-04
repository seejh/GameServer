#pragma once




/*----------------------------------------------------------------
	Global Instance
----------------------------------------------------------------*/

extern class Memory*			GMemory;
extern class ThreadManager*		GThreadManager;
extern class GlobalQueue*		GGlobalQueue;
extern class JobTimer*			GJobTimer;
extern class DeadLockProfiler* GDeadLockProfiler;
extern class DBConnectionPool*	GDBConnectionPool;
extern class ConsoleLog*		GConsoleLogger;


/*--------------------------------------------------------------
	CoreGlobal
---------------------------------------------------------------*/

class CoreGlobal {
public:
	static CoreGlobal* Instance() {
		static CoreGlobal instance;
		return &instance;
	}

	static void Init();
private:
	CoreGlobal();
	CoreGlobal(const CoreGlobal&);
	CoreGlobal& operator=(const CoreGlobal&);
};



/*--------------------------------------------------------------
	TLS (Thread Local Storage)
---------------------------------------------------------------*/
extern thread_local uint32 TLSThreadId;
extern thread_local std::stack<int32> TLSLockStack;
