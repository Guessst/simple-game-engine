#include "constants.h"
#include "collision.h"

#define NULL ((void *)0)

bool PointIsInbounds(float x, float y)
{
    return (0 < x && x < SCREEN_WIDTH && y > 0 && y < SCREEN_HEIGHT);
}

bool PlayerCollidedWithEnemy(Player* player, Enemy** enemyList, unsigned int enemyCount)
{
    unsigned int enemiesChecked = 0;
    unsigned int index = 0;
    
    while(enemiesChecked < enemyCount)
    {
        // TODO: Improve collisions
        Enemy* currEnemy = enemyList[index];

        // DrawRectangleRec(
        //     (Rectangle){
        //             currEnemy->x - ENEMY_RADIUS,
        //             currEnemy->y - ENEMY_RADIUS,
        //             2*ENEMY_RADIUS,
        //             2*ENEMY_RADIUS
        //         },
        //     RED
        // );

        if(
            currEnemy->isActive
            && CheckCollisionPointRec(
                (Vector2){player->x, player->y},
                (Rectangle){
                    currEnemy->x - ENEMY_RADIUS,
                    currEnemy->y - ENEMY_RADIUS,
                    2*ENEMY_RADIUS,
                    2*ENEMY_RADIUS
                }
            )
        ) return true;
        if(currEnemy->isActive) enemiesChecked++;
        index++;
    }
    return false;
}

Enemy* BulletCollidedWithEnemy(Bullet* bullet, Enemy** enemyList, unsigned int enemyCount)
{
    unsigned int enemiesChecked = 0;
    unsigned int index = 0;
    
    while(enemiesChecked < enemyCount)
    {
        // TODO: Improve collisions
        Enemy* currEnemy = enemyList[index];

        // DrawRectangleRec(
        //     (Rectangle){
        //             currEnemy->x - ENEMY_RADIUS,
        //             currEnemy->y - ENEMY_RADIUS,
        //             2*ENEMY_RADIUS,
        //             2*ENEMY_RADIUS
        //         },
        //     RED
        // );

        if(
            currEnemy->isActive
            && CheckCollisionPointRec(
                (Vector2){bullet->x, bullet->y},
                (Rectangle){
                    currEnemy->x - ENEMY_RADIUS,
                    currEnemy->y - ENEMY_RADIUS,
                    2*ENEMY_RADIUS,
                    2*ENEMY_RADIUS
                }
            )
        ) return currEnemy;
        if(currEnemy->isActive) enemiesChecked++;
        index++;
    }

    return NULL;
}