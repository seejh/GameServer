#pragma once


#ifdef _DEBUG

#pragma comment(lib, "MyServerCore\\Debug\\MyServerCore.lib")
#pragma commnet(lib, "Shared\\Debug\\Shared.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#pragma comment(lib, "Hiredis\\Debug\\hiredisd.lib")
#else
#pragma comment(lib, "MyServerCore\\Release\\MyServerCore.lib")
#pragma comment(lib, "Shared\\Release\\Shared.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#pragma comment(lib, "Hiredis\\Release\\hiredis.lib")

#pragma comment(lib, "Recastnavigation\\Release\\DebugUtils.lib")
#pragma comment(lib, "Recastnavigation\\Release\\Detour.lib")
#pragma comment(lib, "Recastnavigation\\Release\\DetourCrowd.lib")
#pragma comment(lib, "Recastnavigation\\Release\\DetourTileCache.lib")
#pragma comment(lib, "Recastnavigation\\Release\\Recast.lib")
#endif


#include<map>

#include"CorePch.h"
#include"Protocol3.pb.h"
#include"DataManager.h"

PROTOCOL::PFVector MakePFVector(float f1, float f2, float f3);