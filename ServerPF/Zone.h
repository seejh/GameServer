#pragma once

class GameObject;
class Player;
class Monster;
class Projectile;
class Npc;
//class Zone
//{
//public:
//	static atomic<int> ZoneIdGenerator;
//
//	Zone();
//	Zone(int baseX, int baseY, int width);
//
//	void Add(shared_ptr<GameObject> object);
//	void Remove(shared_ptr<GameObject> object);
//	//void Find();
//	//void FindAll();
//public:
//	int _zoneId;
//
//	int _baseX;
//	int _baseY;
//	
//	int _maxX;
//	int _minX;
//	int _maxY;
//	int _minY;
//
//	set<shared_ptr<Player>> _players;
//	set<shared_ptr<Monster>> _monsters;
//	set<shared_ptr<Npc>> _npcs;
//	set<shared_ptr<Projectile>> _projectiles;
//};


class Zone {
public:
	Zone();
	void SetZone(int zoneId, int minX, int minY, int zoneWidth, int zoneHeight);

	void Add(shared_ptr<GameObject> gameObject);
	void Remove(shared_ptr<GameObject> gameObject);

public:
	int _zoneId;

	int _minX;
	int _minY;

	int _zoneWidth;
	int _zoneHeight;

	set<shared_ptr<Player>> _players;
	set<shared_ptr<Monster>> _monsters;
	set<shared_ptr<Npc>> _npcs;
};