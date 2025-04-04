#pragma once

#pragma warning(disable:4996)

#include<recastnavigation/Recast.h>
#include<recastnavigation/DetourNavMesh.h>
#include<recastnavigation/DetourNavMeshQuery.h>
#include<recastnavigation/DetourCommon.h>
#include<recastnavigation/DetourPathCorridor.h>

class NavMeshTool {
public:
	NavMeshTool();

	bool LoadNavMesh(const char* filePath);
	bool FindPath(float sp[3], float ep[3], OUT vector<PROTOCOL::PFVector>& paths);
	bool FindPathStraight(float sp[3], float ep[3], OUT vector<PROTOCOL::PFVector>& paths);
	bool FindRandomPos(PROTOCOL::PFVector basePos, float radius, OUT PROTOCOL::PFVector& ep);
	// bool FindRandomPos(dtNavMeshQuery* query, float sp[3], float centerPos[3], float radius, float destPos[3]);

public:
	dtNavMesh* m_navMesh;
	dtQueryFilter m_filter;

	float m_polyPickExt[3];
	queue<dtNavMeshQuery*> _navQueryPool;

private:
	enum {
		MAX_POLYS = 256,
		MAX_SMOOTH = 2048,
	};

	const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
	const int NAVMESHSET_VERSION = 1;

	struct NavMeshSetHeader
	{
		int magic;
		int version;
		int numTiles;
		dtNavMeshParams params;
	};

	struct NavMeshTileHeader
	{
		dtTileRef tileRef;
		int dataSize;
	};

	enum SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
		SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
	};

	//
	inline void Push(dtNavMeshQuery* navQuery);
	inline dtNavMeshQuery* Pop();
	inline dtNavMesh* LoadNavMeshFile(const char* filePath);
	inline void UnrealToRecastCoord(float* pos);
	inline void RecastToUnrealCoord(PROTOCOL::PFVector& pos);


	//
	static float frand() {
		return (float)rand() / (float)RAND_MAX;
	}
	inline bool inRange(const float* v1, const float* v2, const float r, const float h);
	inline int fixupShortcuts(dtPolyRef* path, int npath, dtNavMeshQuery* navQuery);
	inline bool getSteerTarget(dtNavMeshQuery* navQuery, const float* startPos, const float* endPos,
		const float minTargetDist,
		const dtPolyRef* path, const int pathSize,
		float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef,
		float* outPoints = 0, int* outPointCount = 0);
};
