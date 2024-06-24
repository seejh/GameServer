#pragma once

class GameObject;
class Player;
class VisionCube
{
public:
	VisionCube();

	void Gather(set<shared_ptr<GameObject>>& newObjects);
	void Update();

public:
	shared_ptr<Player> _ownerPlayer;

	float _visionDistance;
	set<shared_ptr<GameObject>> _prevObjects;
};

