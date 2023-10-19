#ifndef COLLISION_H_
#define COLLISION_H_

#include "game.h"
#include "constants.h"

bool PointIsInbounds(float x, float y);
bool PlayerCollidedWithEnemy(Player* player, Enemy** enemyList, unsigned int enemyCount);
Enemy* BulletCollidedWithEnemy(Bullet* bullet, Enemy** enemyList, unsigned int enemyCount);

#endif