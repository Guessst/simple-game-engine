#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "raylib.h"

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 800
#define MATTEBLACK CLITERAL(Color){ 40, 40, 43, 255 }      // Matte Black
#define BACKGROUND_COLOR MATTEBLACK

#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT ((PLAYER_WIDTH*1.73205)/2.0f) // Equilateral triangle
#define PLAYER_MOVESPEED 360
#define PLAYER_COLOR { 255, 255, 255, 255 } // WHITE

#define MAX_BULLETS 512
#define BULLET_SPEED (PLAYER_MOVESPEED*2)
#define BULLET_RADIUS 6.0f

#define MAX_ENEMIES 512
#define ENEMY_SPAWNTIME 0.25f
#define ENEMY_SPEED 420
#define ENEMY_ROTATIONSPEED 2.0f
#define ENEMY_RADIUS 40.0f
#define ENEMY_COLOR { 255, 161, 0, 255 } // ORANGE
#define ENEMY_SCORE 100

#define RANDBETWEEN(A, B) ((rand() % (B - A + 1)) + A)

typedef struct Player {
    int width;
    int height;
    float posX;
    float posY;
    Color color;
} Player;

typedef struct Enemy {
    float posX;
    float posY;
    float rotation;
    bool isActive;
    Color color;
} Enemy;

typedef struct Bullet {
    float posX;
    float posY;
    bool isActive;
} Bullet;


// GLOBALS
// ---------------------------------
Player player = {PLAYER_WIDTH, PLAYER_HEIGHT, SCREEN_WIDTH/2.0f, SCREEN_HEIGHT - PLAYER_HEIGHT, PLAYER_COLOR};
long long unsigned score = 0;

Enemy* enemyList[MAX_ENEMIES] = {0};
unsigned int enemyCount = 0;
// Color enemyColors[] = {GOLD, PINK, LIME, BLUE, PURPLE, BROWN};

Bullet* bulletList[MAX_BULLETS] = {0};
unsigned int bulletCount = 0;
// ---------------------------------

void MovePlayer()
{
    float displacement = PLAYER_MOVESPEED * GetFrameTime();

    // TODO: Improve bounds check
    // if(IsKeyDown(KEY_W) && player.posY - displacement > 0) player.posY -= displacement;
    if(IsKeyDown(KEY_A) && player.posX - displacement > 0) player.posX -= displacement;
    // if(IsKeyDown(KEY_S) && player.posY + displacement < SCREEN_HEIGHT) player.posY += displacement;
    if(IsKeyDown(KEY_D) && player.posX + displacement < SCREEN_WIDTH) player.posX += displacement;
}

void DrawPlayer()
{
    Vector2 v1 = { player.posX, player.posY - player.height/2.0f };
    Vector2 v2 = { player.posX - player.width/2.0f, player.posY + player.height/2.0f };
    Vector2 v3 = { player.posX + player.width/2.0f, player.posY + player.height/2.0f };

    DrawTriangle(v1, v2, v3, player.color);
}

bool PlayerCollidedWithEnemy()
{
    unsigned int enemiesChecked = 0;
    unsigned int index = 0;
    
    while(enemiesChecked < enemyCount)
    {
        // TODO: Improve collisions
        Enemy* currEnemy = enemyList[index];

        // DrawRectangleRec(
        //     (Rectangle){
        //             currEnemy->posX - ENEMY_RADIUS,
        //             currEnemy->posY - ENEMY_RADIUS,
        //             2*ENEMY_RADIUS,
        //             2*ENEMY_RADIUS
        //         },
        //     RED
        // );

        if(
            currEnemy->isActive
            && CheckCollisionPointRec(
                (Vector2){player.posX, player.posY},
                (Rectangle){
                    currEnemy->posX - ENEMY_RADIUS,
                    currEnemy->posY - ENEMY_RADIUS,
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

Bullet* InitBullet(float posX, float posY)
{
    Bullet* bullet = (Bullet*) malloc(sizeof(Bullet));
    bullet->posX = posX;
    bullet->posY = posY;
    bullet->isActive = true;

    return bullet;
}

void AddBullet(Bullet* bullet)
{
    bool successAdding = false;
    for(unsigned int i = 0;i < MAX_BULLETS;i++)
    {
        if(bulletList[i] == 0 || !bulletList[i]->isActive)
        {
            if(bulletList[i] != 0) free(bulletList[i]); // overwrites inactive bullet
            bulletList[i] = bullet;
            successAdding = true;
            break;
        }
    }
    assert(successAdding);
    bulletCount++;
}

Enemy* BulletCollidedWithEnemy(Bullet* bullet)
{
    unsigned int enemiesChecked = 0;
    unsigned int index = 0;
    
    while(enemiesChecked < enemyCount)
    {
        // TODO: Improve collisions
        Enemy* currEnemy = enemyList[index];

        // DrawRectangleRec(
        //     (Rectangle){
        //             currEnemy->posX - ENEMY_RADIUS,
        //             currEnemy->posY - ENEMY_RADIUS,
        //             2*ENEMY_RADIUS,
        //             2*ENEMY_RADIUS
        //         },
        //     RED
        // );

        if(
            currEnemy->isActive
            && CheckCollisionPointRec(
                (Vector2){bullet->posX, bullet->posY},
                (Rectangle){
                    currEnemy->posX - ENEMY_RADIUS,
                    currEnemy->posY - ENEMY_RADIUS,
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

// TODO: Break function into 'Update' and 'Draw'
void UpdateAndDrawBullets()
{
    float frametime = GetFrameTime();

    unsigned int bulletsDrawn = 0;
    unsigned int index = 0;
    while(bulletsDrawn < bulletCount)
    {
        Bullet* currBullet = bulletList[index];
        if(currBullet->isActive)
        {
            if(currBullet->posY < 0)
            {
                currBullet->isActive = false;
                bulletCount--;
            }
            else
            {
                Enemy* collidedEnemy = BulletCollidedWithEnemy(currBullet);
                // Collided with enemy
                if(collidedEnemy != NULL)
                {
                    collidedEnemy->isActive = false;
                    enemyCount--;

                    currBullet->isActive = false;
                    bulletCount--;

                    score += ENEMY_SCORE;
                }
                // Didn't collide with enemy
                else
                {
                    currBullet->posY -= BULLET_SPEED * frametime;
                    DrawCircle(currBullet->posX, currBullet->posY, BULLET_RADIUS, WHITE);
                    bulletsDrawn++;
                }
            }
        }
        // else
        // {
        //     DrawCircle(currBullet->posX, currBullet->posY, BULLET_RADIUS, ORANGE);
        // }
        index++;
    }
}

void Shoot()
{
    float initialX = player.posX;
    float initialY = player.posY - player.height/2.0f;
    Bullet* bullet = InitBullet(initialX, initialY);
    AddBullet(bullet);
}

void AddEnemy(Enemy* enemy)
{
    bool successAdding = false;
    for(unsigned int i = 0;i < MAX_ENEMIES;i++)
    {
        if(enemyList[i] == 0 || !enemyList[i]->isActive)
        {
            if(enemyList[i] != 0) free(enemyList[i]); // overwrites inactive enemy
            enemyList[i] = enemy;
            successAdding = true;
            break;
        }
    }
    assert(successAdding);
    enemyCount++;
}

void SpawnEnemy()
{
    Enemy* enemy = (Enemy*) malloc(sizeof(Enemy));
    enemy->posX = RANDBETWEEN((int)ENEMY_RADIUS, SCREEN_WIDTH - (int)ENEMY_RADIUS);
    enemy->posY = -ENEMY_RADIUS;
    enemy->isActive = true;
    // Random values for colors
    enemy->color = (Color){rand() % 255, rand() % 255, rand() % 255, 255};
    // Random colors from global palette
    // enemy->color = enemyColors[(rand() % sizeof(enemyColors[0]))];
    AddEnemy(enemy);
}

// TODO: Break function into 'Update' and 'Draw'
void UpdateAndDrawEnemies()
{
    float frametime = GetFrameTime();

    unsigned int enemiesDrawn = 0;
    unsigned int index = 0;
    while(enemiesDrawn < enemyCount)
    {
        Enemy* currEnemy = enemyList[index];
        if(currEnemy->isActive)
        {
            if(currEnemy->posY > SCREEN_HEIGHT)
            {
                currEnemy->isActive = false;
                enemyCount--;
            }
            else
            {
                currEnemy->posY += ENEMY_SPEED * frametime;
                // currEnemy->rotation += ENEMY_ROTATIONSPEED;

                DrawPoly(
                    (Vector2){ currEnemy->posX, currEnemy->posY },
                    6,
                    ENEMY_RADIUS,
                    currEnemy->rotation,
                    currEnemy->color
                );
                enemiesDrawn++;
            }
        }
        // else
        // {
        //     DrawCircle(currBullet->posX, currBullet->posY, BULLET_RADIUS, ORANGE);
        // }
        index++;
    }
}

int main(void)
{
    // Initialization
    //----------------------------------------------------------------------------------------
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GAME 1");

    SetTargetFPS(60);

    double finalTime;
    float enemyTimer = 0.0f;
    bool isDead = false;
    //----------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //------------------------------------------------------------------------------------
        if(!isDead){
            if(IsKeyPressed(KEY_SPACE)) Shoot();

            MovePlayer();

            enemyTimer += GetFrameTime();
            if(enemyTimer > ENEMY_SPAWNTIME)
            {
                SpawnEnemy();
                enemyTimer = 0;
            }
            
            if(PlayerCollidedWithEnemy())
            {
                isDead = true;
                player.color = RED;
                finalTime = GetTime();
            }
        }
        //------------------------------------------------------------------------------------

        // Draw
        //------------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);

            DrawPlayer();

            UpdateAndDrawBullets();

            UpdateAndDrawEnemies();

            DrawFPS(0, 0);

            DrawText(TextFormat("SCORE %llu", score), 0, 20, 20, WHITE);

            DrawText(TextFormat("TIME %.3lf", isDead ? finalTime : GetTime()), 0, 40, 20, WHITE);
        EndDrawing();
        //------------------------------------------------------------------------------------
    }

    // De-Initialization
    //----------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------------------------------------

    return 0;
}