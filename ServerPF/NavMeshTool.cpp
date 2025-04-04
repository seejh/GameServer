#include "pch.h"
#include "NavMeshTool.h"

/*-----------------------------------------------------------------------------------
	NavMeshTool
------------------------------------------------------------------------------------*/

NavMeshTool::NavMeshTool() : m_navMesh(0) {
	m_filter.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
	m_filter.setExcludeFlags(0);

	m_polyPickExt[0] = 2;
	m_polyPickExt[1] = 4;
	m_polyPickExt[2] = 2;
}

bool NavMeshTool::LoadNavMesh(const char* filePath)
{
	dtFreeNavMesh(m_navMesh);

	m_navMesh = LoadNavMeshFile(filePath);
	if (m_navMesh == nullptr)
		return false;

	return true;
}


bool NavMeshTool::FindPath(float sp[3], float ep[3], OUT vector<PROTOCOL::PFVector>& paths)
{
	if (!m_navMesh)
		return false;

	dtNavMeshQuery* navQuery = Pop();
	if (navQuery == nullptr)
		return false;

	// 단위 변경
	UnrealToRecastCoord(sp);
	UnrealToRecastCoord(ep);

	//
	dtPolyRef startRef;
	dtPolyRef endRef;
	dtPolyRef polys[MAX_POLYS];
	int npolys = 0;
	int nsmoothPath = 0;

	//
	dtStatus sStatus = navQuery->findNearestPoly(sp, m_polyPickExt, &m_filter, &startRef, 0);
	dtStatus eStatus = navQuery->findNearestPoly(ep, m_polyPickExt, &m_filter, &endRef, 0);

	dtStatus pathFindStatus = DT_FAILURE;

	// 경로 탐색
	navQuery->findPath(startRef, endRef, sp, ep, &m_filter, polys, &npolys, MAX_POLYS);
	nsmoothPath = 0;

	// 탐색 후 폴리곤이 있으면
	if (npolys)
	{
		// z 사이드 0.3 고정 체크
		// ext 10, 10, 10으로 고민
		float iterPos[3], targetPos[3];
		navQuery->closestPointOnPoly(startRef, sp, iterPos, 0);
		navQuery->closestPointOnPoly(polys[npolys - 1], ep, targetPos, 0);

		// 스텝 사이즈
		// static const float STEP_SIZE = 0.5f;
		static const float STEP_SIZE = 1.0f;
		static const float SLOP = 0.01f;

		nsmoothPath = 0;

		// 시작 지점 그냥 넣음
		// dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);

		PROTOCOL::PFVector location;
		location.set_x(iterPos[0]);
		location.set_y(iterPos[1]);
		location.set_z(iterPos[2]);

		RecastToUnrealCoord(location);
		paths.push_back(location);
		nsmoothPath++;

		// Move towards target a small advancement at a time until target reached or
		// when ran out of memory to store the path.
		// 루프 스타트 지점, 시작 지점에서 도착 지점까지 짧은 시간 단위로 이동
		while (npolys && nsmoothPath < MAX_SMOOTH)
		{
			// Find location to steer towards.
			float steerPos[3];
			unsigned char steerPosFlag;
			dtPolyRef steerPosRef;

			// 꺽이는 지점을 찾는다, 없으면 종점 위치로 설정된다.
			// 이게 없으면 break인데, 그 말은 갈 수 있는 경로가 없다는 뜻
			if (!getSteerTarget(navQuery, iterPos, targetPos, SLOP,
				polys, npolys, steerPos, steerPosFlag, steerPosRef))
				break;

			bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
			bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

			// Find movement delta.
			float delta[3], len;
			dtVsub(delta, steerPos, iterPos);
			len = dtMathSqrtf(dtVdot(delta, delta));
			// If the steer target is end of path or off-mesh link, do not move past the location.
			if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
				len = 1;
			else
				len = STEP_SIZE / len;
			float moveTgt[3];
			dtVmad(moveTgt, iterPos, delta, len);

			// 이동, moveTgt -> result -> iterPos
			float result[3];
			dtPolyRef visited[16];
			int nvisited = 0;
			navQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &m_filter,
				result, visited, &nvisited, 16);

			npolys = dtMergeCorridorStartMoved(polys, npolys, MAX_POLYS, visited, nvisited);
			npolys = fixupShortcuts(polys, npolys, navQuery);

			float h = 0;
			navQuery->getPolyHeight(polys[0], result, &h);
			result[1] = h;
			dtVcopy(iterPos, result);

			// Handle end of path and off-mesh links when close enough.
			// 종점 도착 - 마지막 직선 주로 && 충분히 가까이 있다면
			if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
			{
				// Reached end of path.
				dtVcopy(iterPos, targetPos);
				if (nsmoothPath < MAX_SMOOTH)
				{
					// dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);
					location.set_x(iterPos[0]);
					location.set_y(iterPos[1]);
					location.set_z(iterPos[2]);
					RecastToUnrealCoord(location);
					paths.push_back(location);

					nsmoothPath++;
				}

				int idx = (nsmoothPath - 1) * 3;

				pathFindStatus = DT_SUCCESS;

				break;
			}
			// 메쉬 간 경계선 도착
			else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
			{
				// Reached off-mesh connection.
				float startPos[3], endPos[3];

				// Advance the path up to and over the off-mesh connection.
				dtPolyRef prevRef = 0, polyRef = polys[0];
				int npos = 0;
				while (npos < npolys && polyRef != steerPosRef)
				{
					prevRef = polyRef;
					polyRef = polys[npos];
					npos++;
				}
				for (int i = npos; i < npolys; ++i)
					polys[i - npos] = polys[i];
				npolys -= npos;

				// Handle the connection.
				dtStatus status = m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
				if (dtStatusSucceed(status))
				{
					if (nsmoothPath < MAX_SMOOTH)
					{
						//dtVcopy(&smoothPath[nsmoothPath * 3], startPos);
						location.set_x(startPos[0]);
						location.set_y(startPos[1]);
						location.set_z(startPos[2]);
						RecastToUnrealCoord(location);
						paths.push_back(location);
						nsmoothPath++;
						// Hack to make the dotted path not visible during off-mesh connection.
						if (nsmoothPath & 1)
						{
							// dtVcopy(&smoothPath[nsmoothPath * 3], startPos);
							location.set_x(startPos[0]);
							location.set_y(startPos[1]);
							location.set_z(startPos[2]);
							nsmoothPath++;
						}
					}
					// Move position at the other side of the off-mesh link.
					dtVcopy(iterPos, endPos);
					float eh = 0.0f;
					navQuery->getPolyHeight(polys[0], iterPos, &eh);
					iterPos[1] = eh;
				}
			}

			// Store results.
			// 중간 중간 단계별 저장
			if (nsmoothPath < MAX_SMOOTH)
			{
				// dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);
				nsmoothPath++;

				//
				location.set_x(iterPos[0]);
				location.set_y(iterPos[1]);
				location.set_z(iterPos[2]);
				RecastToUnrealCoord(location);
				paths.push_back(location);
			}
		} // while 끝
	} // FindPath 후 폴리곤이 있으면 끝
	// 폴리곤이 없음
	else {

	}

	// 반납
	_navQueryPool.push(navQuery);

	return dtStatusSucceed(pathFindStatus);
}

bool NavMeshTool::FindPathStraight(float sp[3], float ep[3], OUT vector<PROTOCOL::PFVector>& paths)
{
	if (!m_navMesh)
		return false;

	dtNavMeshQuery* navQuery = Pop();
	if (navQuery == nullptr)
		return false;

	cout << "SP - [" << sp[0] << ", " << sp[1] << ", " << sp[2] << "]" << endl;
	cout << "EP - [" << ep[0] << ", " << ep[1] << ", " << ep[2] << "]" << endl;

	// 단위 변경
	UnrealToRecastCoord(sp);
	UnrealToRecastCoord(ep);


	dtPolyRef startRef;
	dtPolyRef endRef;
	dtPolyRef polys[MAX_POLYS];
	int npolys = 0;
	int nstraightPath = 0;
	float straightPath[MAX_POLYS * 3];
	unsigned char straightPathFlags[MAX_POLYS];
	dtPolyRef straightPathPolys[MAX_POLYS];
	int straightPathOptions = 0;

	navQuery->findNearestPoly(sp, m_polyPickExt, &m_filter, &startRef, 0);
	navQuery->findNearestPoly(ep, m_polyPickExt, &m_filter, &endRef, 0);

	if (startRef == 0 || endRef == 0)
		return false;

	navQuery->findPath(startRef, endRef, sp, ep, &m_filter, polys, &npolys, MAX_POLYS);
	if (npolys) {

		float epos[3];
		dtVcopy(epos, ep);
		if (polys[npolys - 1] != endRef)
			navQuery->closestPointOnPoly(polys[npolys - 1], ep, epos, 0);

		navQuery->findStraightPath(sp, epos, polys, npolys, straightPath, straightPathFlags,
			straightPathPolys, &nstraightPath, MAX_POLYS, straightPathOptions);

		// 
		PROTOCOL::PFVector path;
		for (int i = 0; i < nstraightPath; i = i + 3) {
			path.set_x(straightPath[i]);
			path.set_y(straightPath[i + 1]);
			path.set_z(straightPath[i + 2]);
			RecastToUnrealCoord(path);
			paths.push_back(path);
		}
	}

	return true;
}

bool NavMeshTool::FindRandomPos(PROTOCOL::PFVector basePos, float radius, OUT PROTOCOL::PFVector& ep)
{
	// 준비
	dtPolyRef baseRef;
	dtPolyRef destRef;
	float fbasePos[3] = { basePos.x(), basePos.y(), basePos.z() };
	float destPos[3];

	// 단위 변환 - 좌표 형식 (언리얼 -> 리캐스트), radius (언리얼 -> 리캐스트)
	UnrealToRecastCoord(fbasePos);
	// radius *= 0.8f;
	radius /= 100.f;

	cout << "- NavMesh -" << endl;
	cout << "basePos:" << fbasePos[0] << ", " << fbasePos[1] << ", " << fbasePos[2] << endl;
	cout << "radius : " << radius << endl;


	// 쿼리 추출
	dtNavMeshQuery* query = Pop();
	if (query == nullptr)
		return false;

	// 중심점이 해당하는 폴리곤
	// dtStatus findPolyStatus = query->findNearestPoly(fbasePos, &radius, &m_filter, &baseRef, 0);
	dtStatus findPolyStatus = query->findNearestPoly(fbasePos, m_polyPickExt, &m_filter, &baseRef, 0);
	if (dtStatusSucceed(findPolyStatus) == false) {
		return false;
	}

	// 램덤 포스 조회
	dtStatus findRandomStatus = query->findRandomPointAroundCircle(baseRef, fbasePos, radius, &m_filter, &NavMeshTool::frand, &destRef, destPos);
	if (dtStatusSucceed(findRandomStatus) == false) {
		return false;
	}

	cout << "randomPos:" << destPos[0] << ", " << destPos[1] << ", " << destPos[2] << endl;

	// 좌표 형식 (리캐스트 -> 언리얼)
	ep.set_x(destPos[0]);
	ep.set_y(destPos[1]);
	ep.set_z(destPos[2]);
	RecastToUnrealCoord(ep);

	// 쿼리 반납
	Push(query);

	return true;
}

/*--------------------------------------------------------------------------
	Inline
---------------------------------------------------------------------------------*/

inline void NavMeshTool::Push(dtNavMeshQuery* navQuery)
{
	if (navQuery != nullptr) {
		_navQueryPool.push(navQuery);
	}
}

inline dtNavMeshQuery* NavMeshTool::Pop()
{
	dtNavMeshQuery* navQuery = nullptr;

	if (_navQueryPool.empty() == true) {
		navQuery = dtAllocNavMeshQuery();

		dtStatus status = navQuery->init(m_navMesh, 2048);
		if (dtStatusSucceed(status) == false) {
			cout << "Error" << endl;
			return nullptr;
		}
	}

	else {
		navQuery = _navQueryPool.front();
		_navQueryPool.pop();
	}

	return navQuery;
}

inline dtNavMesh* NavMeshTool::LoadNavMeshFile(const char* filePath)
{
	//
	FILE* fp = fopen(filePath, "rb");
	if (!fp) return 0;

	// Read header.
	NavMeshSetHeader header;
	size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
	if (readLen != 1)
	{
		fclose(fp);
		return 0;
	}
	if (header.magic != NAVMESHSET_MAGIC)
	{
		fclose(fp);
		return 0;
	}
	if (header.version != NAVMESHSET_VERSION)
	{
		fclose(fp);
		return 0;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		fclose(fp);
		return 0;
	}
	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return 0;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			fclose(fp);
			return 0;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	fclose(fp);

	return mesh;
}

inline void NavMeshTool::UnrealToRecastCoord(float* pos)
{
	// 부호 변경
	pos[0] *= -1;
	pos[1] *= -1;

	// 자리 변경
	float tmp = pos[1];
	pos[1] = pos[2];
	pos[2] = tmp;

	// 100배 축소
	pos[0] /= 100;
	pos[1] /= 100;
	pos[2] /= 100;
}

inline void NavMeshTool::RecastToUnrealCoord(PROTOCOL::PFVector& pos)
{
	float tmpArray[3];
	tmpArray[0] = pos.x();
	tmpArray[1] = pos.y();
	tmpArray[2] = pos.z();

	// 부호 변경
	tmpArray[0] *= -1;
	tmpArray[2] *= -1;

	// 자리 변경
	float tmp = tmpArray[1];
	tmpArray[1] = tmpArray[2];
	tmpArray[2] = tmp;

	// 100배 확대
	tmpArray[0] *= 100;
	tmpArray[1] *= 100;
	tmpArray[2] *= 100;

	// 이동
	pos.set_x(tmpArray[0]);
	pos.set_y(tmpArray[1]);
	pos.set_z(tmpArray[2]);
}

/*--------------------------------------------------------------------

--------------------------------------------------------------------*/
inline bool NavMeshTool::inRange(const float* v1, const float* v2, const float r, const float h)
{
	const float dx = v2[0] - v1[0];
	const float dy = v2[1] - v1[1];
	const float dz = v2[2] - v1[2];

	return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
}
inline int NavMeshTool::fixupShortcuts(dtPolyRef* path, int npath, dtNavMeshQuery* navQuery)
{
	if (npath < 3)
		return npath;

	// Get connected polygons
	static const int maxNeis = 16;
	dtPolyRef neis[maxNeis];
	int nneis = 0;

	const dtMeshTile* tile = 0;
	const dtPoly* poly = 0;
	if (dtStatusFailed(navQuery->getAttachedNavMesh()->getTileAndPolyByRef(path[0], &tile, &poly)))
		return npath;

	for (unsigned int k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
	{
		const dtLink* link = &tile->links[k];
		if (link->ref != 0)
		{
			if (nneis < maxNeis)
				neis[nneis++] = link->ref;
		}
	}

	// If any of the neighbour polygons is within the next few polygons
	// in the path, short cut to that polygon directly.
	static const int maxLookAhead = 6;
	int cut = 0;
	for (int i = dtMin(maxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
		for (int j = 0; j < nneis; j++)
		{
			if (path[i] == neis[j]) {
				cut = i;
				break;
			}
		}
	}
	if (cut > 1)
	{
		int offset = cut - 1;
		npath -= offset;
		for (int i = 1; i < npath; i++)
			path[i] = path[i + offset];
	}

	return npath;
}
inline bool NavMeshTool::getSteerTarget(dtNavMeshQuery* navQuery, const float* startPos, const float* endPos, const float minTargetDist, const dtPolyRef* path, const int pathSize, float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef, float* outPoints, int* outPointCount)
{
	// Find steer target.
	static const int MAX_STEER_POINTS = 3;
	float steerPath[MAX_STEER_POINTS * 3];
	unsigned char steerPathFlags[MAX_STEER_POINTS];
	dtPolyRef steerPathPolys[MAX_STEER_POINTS];
	int nsteerPath = 0;
	navQuery->findStraightPath(startPos, endPos, path, pathSize,
		steerPath, steerPathFlags, steerPathPolys, &nsteerPath, MAX_STEER_POINTS);
	if (!nsteerPath)
		return false;

	if (outPoints && outPointCount)
	{
		*outPointCount = nsteerPath;
		for (int i = 0; i < nsteerPath; ++i)
			dtVcopy(&outPoints[i * 3], &steerPath[i * 3]);
	}


	// Find vertex far enough to steer to.
	int ns = 0;
	while (ns < nsteerPath)
	{
		// Stop at Off-Mesh link or when point is further than slop away.
		if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
			!inRange(&steerPath[ns * 3], startPos, minTargetDist, 1000.0f))
			break;
		ns++;
	}
	// Failed to find good point to steer to.
	if (ns >= nsteerPath)
		return false;

	dtVcopy(steerPos, &steerPath[ns * 3]);
	steerPos[1] = startPos[1];
	steerPosFlag = steerPathFlags[ns];
	steerPosRef = steerPathPolys[ns];

	return true;
}
