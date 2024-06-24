#pragma once


#ifdef _DEBUG

#pragma comment(lib, "MyServerCore\\Debug\\MyServerCore.lib")

#pragma comment(lib, "libprotobufd.lib")
#else
#pragma comment(lib, "MyServerCore\\Release\\MyServerCore.lib")
#pragma comment(lib, "libprotobuf.lib")
//#pragma comment(lib, "Protobuf\\Release\\libprotobuf .lib")
#endif


#include<map>

#include"CorePch.h"
#include"Protocol3.pb.h"
#include"DataManager.h"


#include"rapidjson/document.h"
#include"rapidjson/writer.h"
#include"rapidjson/stringbuffer.h"

