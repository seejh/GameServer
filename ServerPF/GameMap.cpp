#include "pch.h"
#include "GameMap.h"

#include"Zone.h"
#include"JsonParser.h"
#include"NavMeshTool.h"
#include"GameObject.h"

bool GameMap::Load()
{
	// Config Load
	_minX = -500;
	_maxX = 9500;
	_minY = -6500;
	_maxY = 500;
	
	_mapId = 1;
	_zoneId = 0;
	
	_zoneWidth = 1000;
	_zoneHeight = 1000;

	// 구획(Zone) 생성
	// width, height 개수
	_zoneWidthCount = abs(_maxX - _minX) / _zoneWidth;
	_zoneHeightCount = abs(_maxY - _minY) / _zoneHeight;

	_zones.resize(_zoneHeightCount);
	for (int h = 0; h < _zoneHeightCount; h++) {

		_zones[h].resize(_zoneWidthCount);
		for (int w = 0; w < _zoneWidthCount; w++) {

			int nowZoneMinX = _zoneWidth * w;
			int nowZoneMinY = _zoneHeight * h;

			shared_ptr<Zone> zone = make_shared<Zone>();
			zone->SetZone(_zoneId++, nowZoneMinX, nowZoneMinY, _zoneWidth, _zoneHeight);
			_zones[h][w] = zone;
		}
	}

	const char* mapFile = "solo_navmesh.bin";

	_tool = new NavMeshTool();
	return _tool->LoadNavMesh(mapFile);
}

bool GameMap::FindPath(shared_ptr<Monster> monster, PROTOCOL::PFVector ep, vector<PROTOCOL::PFVector>& paths)
{
	float fsp[3] = { monster->_info.pos().location().x(), monster->_info.pos().location().y(), monster->_info.pos().location().z() };
	float fep[3] = { ep.x(), ep.y(), ep.z() };

	return _tool->FindPath(fsp, fep, paths);
}

bool GameMap::FindRandomPos(shared_ptr<Monster> monster, PROTOCOL::PFVector& destPos)
{
	if (_tool->FindRandomPos(monster->_basePos, monster->_info.stat().returndistance(), destPos) == false)
		return false;

	cout << "- Unreal -" << endl;
	cout << "basePos:" << monster->_basePos.x() << ", " << monster->_basePos.y() << ", " << monster->_basePos.z() << endl;
	cout << "returndistance:" << monster->_info.stat().returndistance() << endl;
	cout << "randomPos:" << destPos.x() << ", " << destPos.y() << ", " << destPos.z() << endl;

	return true;
}


