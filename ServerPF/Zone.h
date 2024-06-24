#pragma once

class GameObject;
class Player;
class Monster;
class Projectile;
class Zone
{
public:
	static atomic<int> ZoneIdGenerator;

	Zone();
	Zone(int baseX, int baseY, int width);

	void Add(shared_ptr<GameObject> object);
	void Remove(shared_ptr<GameObject> object);
	//void Find();
	//void FindAll();
public:
	int _zoneId;

	int _baseX;
	int _baseY;
	
	int _maxX;
	int _minX;
	int _maxY;
	int _minY;

	set<shared_ptr<Player>> _players;
	set<shared_ptr<Monster>> _monsters;
	set<shared_ptr<Projectile>> _projectiles;
};
