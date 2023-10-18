#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "raylib.h"
#include "raymath.h"

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 800
#define MATTEBLACK CLITERAL(Color){ 40, 40, 43, 255 }      // Matte Black
#define BACKGROUND_COLOR MATTEBLACK

#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT ((PLAYER_WIDTH*1.73205)/2.0f) // Equilateral triangle
#define PLAYER_MOVESPEED 360
#define PLAYER_ROTATESPEED 300.0f
#define PLAYER_RELOADSPEED 0.25f
#define PLAYER_COLOR { 255, 255, 255, 255 } // WHITE

#define MAX_BULLETS 512
#define BULLET_SPEED (PLAYER_MOVESPEED*2)
#define BULLET_RADIUS 6.0f

#define MAX_ENEMIES 512
#define ENEMY_SPAWNTIME 0.25f
#define ENEMY_SPEED 360
#define ENEMY_ROTATIONSPEED 2.0f
#define ENEMY_RADIUS 35.0f
#define ENEMY_COLOR { 255, 161, 0, 255 } // ORANGE
#define ENEMY_SCORE 100

#define RANDBETWEEN(A, B) ((rand() % (B - A + 1)) + A)
#define MAX(A, B) (A > B) ? A : B
#define MIN(A, B) (A < B) ? A : B

#define UP (Vector2){0, -1}

typedef struct Player {
    int width;
    int height;
    float x;
    float y;
    bool isReloading;
    float rotation;
    Color color;
} Player;

typedef struct Enemy {
    float x;
    float y;
    float rotation;
    bool hasEnteredScreen;
    bool isActive;
    Color color;
    Vector2 direction;
} Enemy;

typedef struct Bullet {
    float x;
    float y;
    bool isActive;
    Vector2 direction;
} Bullet;


// GLOBALS
// ---------------------------------
Player player = {PLAYER_WIDTH, PLAYER_HEIGHT, SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f, false, 0.0f, PLAYER_COLOR};
long long unsigned score = 0;

Enemy* enemyList[MAX_ENEMIES] = {0};
unsigned int enemyCount = 0;
// Color enemyColors[] = {GOLD, PINK, LIME, BLUE, PURPLE, BROWN};

Bullet* bulletList[MAX_BULLETS] = {0};
unsigned int bulletCount = 0;
// ---------------------------------

bool PointIsInbounds(float x, float y)
{
    return (0 < x && x < SCREEN_WIDTH && y > 0 && y < SCREEN_HEIGHT);
}

void MovePlayerWithKeyboard()
{
    Vector2 direction = Vector2Zero();
    float movementX;
    float movementY;
    float displacement = PLAYER_MOVESPEED * GetFrameTime();

    if(IsKeyDown(KEY_W)) direction.y -= 1;
    if(IsKeyDown(KEY_A)) direction.x -= 1;
    if(IsKeyDown(KEY_S)) direction.y += 1;
    if(IsKeyDown(KEY_D)) direction.x += 1;

    direction = Vector2Normalize(direction);
    movementX = direction.x * displacement;
    movementY = direction.y * displacement;

    if(PointIsInbounds(player.x + movementX, player.y + movementY))
    {
        player.x += movementX;
        player.y += movementY;
    }
}

void MovePlayerWithGamepad()
{
    Vector2 direction = Vector2Zero();
    float movementX;
    float movementY;
    float displacement = PLAYER_MOVESPEED * GetFrameTime();

    float leftAxisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
    float leftAxisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
    
    direction.x = leftAxisX;
    direction.y = leftAxisY;

    // direction = Vector2Normalize(direction);
    movementX = direction.x * displacement;
    movementY = direction.y * displacement;

    if(PointIsInbounds(player.x + movementX, player.y + movementY))
    {
        player.x += movementX;
        player.y += movementY;
    }
}

void RotatePlayerWithKeyboard()
{
    float rotation = PLAYER_ROTATESPEED * GetFrameTime();

    if(IsKeyDown(KEY_LEFT)) player.rotation -= rotation;
    if(IsKeyDown(KEY_RIGHT)) player.rotation += rotation;

    if(player.rotation <= -360.0f || player.rotation >= 360.0f) player.rotation = 0.0f;
}

void RotatePlayerWithGamepad()
{

    float rightAxisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
    float rightAxisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

    Vector2 direction = (Vector2){rightAxisX, rightAxisY};
    float rotation = Vector2Angle(direction, UP)*RAD2DEG;

    player.rotation = rotation;
}

void DrawPlayer()
{
    DrawPoly(
        (Vector2){player.x, player.y},
        3,
        player.width/2.0f,
        player.rotation + 30.0f,
        player.color
    );
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
                (Vector2){player.x, player.y},
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

Bullet* InitBullet(float posX, float posY, Vector2 direction)
{
    Bullet* bullet = (Bullet*) malloc(sizeof(Bullet));
    bullet->x = posX;
    bullet->y = posY;
    bullet->direction = direction;
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
            if(!PointIsInbounds(currBullet->x, currBullet->y))
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
                    currBullet->x += currBullet->direction.x * BULLET_SPEED * frametime;
                    currBullet->y += currBullet->direction.y * BULLET_SPEED * frametime;
                    // currBullet->y -= BULLET_SPEED * frametime;
                    DrawCircle(currBullet->x, currBullet->y, BULLET_RADIUS, WHITE);
                    bulletsDrawn++;
                }
            }
        }
        // else
        // {
        //     DrawCircle(currBullet.x, currBullet.y, BULLET_RADIUS, ORANGE);
        // }
        index++;
    }
}

void Shoot()
{
    // TODO: Bullet are currently spawning from the center,
    // eventually they should come from the tips.
    /*
    
        b2
        /\
       /  \
      /____\
    b1      b3
    */
    float initialX = player.x;
    float initialY = player.y;
    Vector2 direction = Vector2Rotate(UP, player.rotation*DEG2RAD);
    
    Bullet* b1 = InitBullet(initialX, initialY, Vector2Rotate(direction, -120.0f*DEG2RAD));
    AddBullet(b1);

    Bullet* b2 = InitBullet(initialX, initialY, direction);
    AddBullet(b2);

    Bullet* b3 = InitBullet(initialX, initialY, Vector2Rotate(direction, 120.0f*DEG2RAD));
    AddBullet(b3);

    player.isReloading = true;
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

    // Enemies spawn in a circle inside the screen
    int radius = ((MAX(SCREEN_WIDTH, SCREEN_HEIGHT)) / 2) - ENEMY_RADIUS;
    float rotation = rand() % 360;
    float angle = rotation*DEG2RAD;
    float x = SCREEN_WIDTH/2 + (cosf(angle) * radius);
    float y = SCREEN_HEIGHT/2 + (sinf(angle) * radius);
    // DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, x, y, GREEN);

    enemy->x = x;
    enemy->y = y;
    enemy->hasEnteredScreen = false;
    enemy->isActive = true;
    enemy->direction =
        Vector2Normalize((Vector2){
                            (player.x - enemy->x),
                            player.y - enemy->y}
                        );
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
            if(currEnemy->hasEnteredScreen && !PointIsInbounds(currEnemy->x, currEnemy->y))
            {
                currEnemy->isActive = false;
                enemyCount--;
            }
            else
            {
                currEnemy->hasEnteredScreen = true;
                currEnemy->x += currEnemy->direction.x * ENEMY_SPEED * frametime;
                currEnemy->y += currEnemy->direction.y * ENEMY_SPEED * frametime;
                // currEnemy->y += ENEMY_SPEED * frametime;
                // currEnemy->rotation += ENEMY_ROTATIONSPEED;

                DrawPoly(
                    (Vector2){ currEnemy->x, currEnemy->y },
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
        //     DrawCircle(currBullet.x, currBullet.y, BULLET_RADIUS, ORANGE);
        // }
        index++;
    }
}

int main(void)
{
    // Initialization
    //----------------------------------------------------------------------------------------
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GAME 2");

    SetTargetFPS(1000);

    double finalTime;
    float enemyTimer = 0.0f;
    float playerReloadTimer = 0.0f;
    bool isDead = false;
    //----------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //------------------------------------------------------------------------------------
        if(!isDead){
            if(IsGamepadAvailable(0))
            {
                if(
                    (GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) > -1)
                    && !player.isReloading
                ) Shoot();
                MovePlayerWithGamepad();
                RotatePlayerWithGamepad();
            }
            else
            {
                if(
                    IsKeyDown(KEY_SPACE)
                    && !player.isReloading
                ) Shoot();
                MovePlayerWithKeyboard();
                RotatePlayerWithKeyboard();
            }

            enemyTimer += GetFrameTime();
            if(enemyTimer > ENEMY_SPAWNTIME)
            {
                SpawnEnemy();
                enemyTimer = 0;
            }

            if(player.isReloading)
            {
                playerReloadTimer += GetFrameTime();
                if(playerReloadTimer >= PLAYER_RELOADSPEED)
                {
                    player.isReloading = false;
                    playerReloadTimer = 0.0f;
                }
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

            DrawText(TextFormat("ROTATION %.3f", player.rotation), 0, 60, 20, BLUE);

            DrawText(TextFormat("BULLETS %u", bulletCount), 0, 80, 20, BLUE);

            DrawText(TextFormat("ENEMIES %u", enemyCount), 0, 100, 20, BLUE);
        EndDrawing();
        //------------------------------------------------------------------------------------
    }

    // De-Initialization
    //----------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------------------------------------

    return 0;
}