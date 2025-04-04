#pragma once

class NavMeshTool;
class Player;
class Monster;
class Npc;
class Zone;
class GameMap
{
public:
	bool Load();

	bool FindPath(shared_ptr<Monster> monster, PROTOCOL::PFVector ep, vector<PROTOCOL::PFVector>& paths);
	bool FindRandomPos(shared_ptr<Monster> monster, PROTOCOL::PFVector& destPos);

public:
	int _zoneId = 0;

	int _mapId;

	int _minX;
	int _minY;
	int _maxX;
	int _maxY;

	int _zoneWidth;
	int _zoneHeight;

	int _zoneWidthCount;
	int _zoneHeightCount;

	vector<vector<shared_ptr<Zone>>> _zones;

	map<int, shared_ptr<Player>> _players;
	map<int, shared_ptr<Monster>> _monsters;
	map<int, shared_ptr<Npc>> _npcs;

	NavMeshTool* _tool;
};

