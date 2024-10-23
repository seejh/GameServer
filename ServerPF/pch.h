#pragma once


#ifdef _DEBUG

#pragma comment(lib, "MyServerCore\\Debug\\MyServerCore.lib")
#pragma comment(lib, "Protobuf\\Debug\\libprotobufd.lib")
#pragma comment(lib, "Hiredis\\Debug\\hiredisd.lib")
#else
#pragma comment(lib, "MyServerCore\\Release\\MyServerCore.lib")
#pragma comment(lib, "Protobuf\\Release\\libprotobuf.lib")
#pragma comment(lib, "Hiredis\\Release\\hiredis.lib")
#endif


#include<map>

#include"CorePch.h"
#include"Protocol3.pb.h"
#include"DataManager.h"


#include"rapidjson/document.h"
#include"rapidjson/writer.h"
#include"rapidjson/stringbuffer.h"

