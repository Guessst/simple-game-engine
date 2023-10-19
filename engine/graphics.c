#include "graphics.h"

void DrawPlayer(Player* player)
{
    DrawPoly(
        (Vector2){ player->x, player->y },
        3,
        player->width/2.0f,
        player->rotation + 30.0f,
        player->color
    );
}

void DrawBullet(Bullet* bullet)
{
    DrawCircle(bullet->x, bullet->y, BULLET_RADIUS, WHITE);
}

void DrawEnemy(Enemy* enemy)
{
    DrawPoly(
        (Vector2){ enemy->x, enemy->y },
        6,
        ENEMY_RADIUS,
        enemy->rotation,
        enemy->color
    );
}