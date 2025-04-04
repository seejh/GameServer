#include "pch.h"
#include "Zone.h"

#include"GameObject.h"
#include"ObjectManager.h"



//atomic<int> Zone::ZoneIdGenerator = 0;
//
//Zone::Zone() {
//    _zoneId = ZoneIdGenerator.fetch_add(1);
//}
//
//Zone::Zone(int baseX, int baseY, int width) : _baseX(baseX), _baseY(baseY)
//{
//    _zoneId = ZoneIdGenerator.fetch_add(1);
//
//    _maxX = _baseX + width - 1;
//    _minX = _baseX;
//    _maxY = _baseY + width - 1;
//    _minY = _baseY;
//}
//

Zone::Zone()
{
}

void Zone::SetZone(int zoneId, int minX, int minY, int zoneWidth, int zoneHeight)
{
    // 
    _zoneId = zoneId;
    _minX = minX;
    _minY = minY;
    _zoneWidth = zoneWidth;
    _zoneHeight = zoneHeight;

    //

}

void Zone::Add(shared_ptr<GameObject> gameObject)
{
    if (gameObject) {
        PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(gameObject->_info.objectid());
        if (type == PROTOCOL::GameObjectType::PLAYER) {
            _players.insert(static_pointer_cast<Player>(gameObject));
        }
        else if (type == PROTOCOL::GameObjectType::MONSTER) {
            _monsters.insert(static_pointer_cast<Monster>(gameObject));
        }
        else {

        }
    }
}

void Zone::Remove(shared_ptr<GameObject> gameObject)
{
    if (gameObject) {
        PROTOCOL::GameObjectType type = ObjectManager::Instance()->GetObjectTypeById(gameObject->_info.objectid());
        if (type == PROTOCOL::GameObjectType::PLAYER) {
            _players.erase(static_pointer_cast<Player>(gameObject));
        }
        else if (type == PROTOCOL::GameObjectType::MONSTER) {
            _monsters.erase(static_pointer_cast<Monster>(gameObject));
        }
        else {

        }
    }
}
