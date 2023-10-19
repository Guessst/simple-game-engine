#include <stdlib.h>
#include <assert.h>
#include "game.h"
#include "graphics.h"
#include "collision.h"
#include "raymath.h"

// GLOBALS
// ---------------------------------
Player player = {PLAYER_WIDTH, PLAYER_HEIGHT, SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f, false, 0.0f, PLAYER_COLOR};
bool playerIsDead = false;
float playerReloadTimer = 0.0f;
long long unsigned score = 0;

Enemy* enemyList[MAX_ENEMIES] = {0};
unsigned int enemyCount = 0;
float enemyTimer = 0.0f;
// Color enemyColors[] = {GOLD, PINK, LIME, BLUE, PURPLE, BROWN};

Bullet* bulletList[MAX_BULLETS] = {0};
unsigned int bulletCount = 0;

double finalTime = 0;
// ---------------------------------

void MovePlayerWithKeyboard(void)
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

void MovePlayerWithGamepad(void)
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

void RotatePlayerWithKeyboard(void)
{
    float rotation = PLAYER_ROTATESPEED * GetFrameTime();

    if(IsKeyDown(KEY_LEFT)) player.rotation -= rotation;
    if(IsKeyDown(KEY_RIGHT)) player.rotation += rotation;

    if(player.rotation <= -360.0f || player.rotation >= 360.0f) player.rotation = 0.0f;
}

void RotatePlayerWithGamepad(void)
{

    float rightAxisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
    float rightAxisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

    Vector2 direction = (Vector2){rightAxisX, rightAxisY};
    float rotation = Vector2Angle(direction, UP)*RAD2DEG;

    player.rotation = rotation;
}

void Shoot(void)
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

void UpdatePlayer(void)
{
    if(!playerIsDead){
        // GAMEPAD CONTROLS IF AVAILABLE
        if(IsGamepadAvailable(0))
        {
            if(
                (GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) > -1)
                && !player.isReloading
            ) Shoot();
            MovePlayerWithGamepad();
            RotatePlayerWithGamepad();
        }
        // KEYBOARD CONTROLS
        if(
            IsKeyDown(KEY_SPACE)
            && !player.isReloading
        ) Shoot();
        MovePlayerWithKeyboard();
        RotatePlayerWithKeyboard();
        
        if(PlayerCollidedWithEnemy(&player, &enemyList, enemyCount))
        {
            playerIsDead = true;
            player.color = RED;
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
    }
    else
    {
        if(
            IsGamepadAvailable(0)
            && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
        {
            ResetGame();
        }
        if(IsKeyPressed(KEY_R)) ResetGame();
    }
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

void SpawnEnemy(void)
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

void UpdateEnemies(void)
{
    float frametime = GetFrameTime();
    unsigned int enemiesUpdated = 0;
    unsigned int index = 0;

    // SPAWNING
    enemyTimer += frametime;
    if(enemyTimer > ENEMY_SPAWNTIME)
    {
        SpawnEnemy();
        enemyTimer = 0;
    }

    // MOVEMENT
    while(enemiesUpdated < enemyCount)
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

                enemiesUpdated++;
            }
        }
        // else
        // {
        //     DrawCircle(currBullet.x, currBullet.y, BULLET_RADIUS, ORANGE);
        // }
        index++;
    }
}

void DrawEnemies(void)
{
    unsigned int enemiesDrawn = 0;
    unsigned int index = 0;
    while(enemiesDrawn < enemyCount)
    {
        Enemy* currEnemy = enemyList[index];
        if(currEnemy->isActive)
        {
            DrawEnemy(currEnemy);
            enemiesDrawn++;
        }
        index++;
    }
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

void UpdateBullets(void)
{
    float frametime = GetFrameTime();

    unsigned int bulletsUpdated = 0;
    unsigned int index = 0;
    while(bulletsUpdated < bulletCount)
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
                Enemy* collidedEnemy = BulletCollidedWithEnemy(currBullet, &enemyList, enemyCount);
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
                    
                    bulletsUpdated++;
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

void DrawBullets(void)
{
    unsigned int bulletsDrawn = 0;
    unsigned int index = 0;
    while(bulletsDrawn < bulletCount)
    {
        Bullet* currBullet = bulletList[index];
        if(currBullet->isActive)
        {
            DrawBullet(currBullet);
            bulletsDrawn++;
        }
        index++;
    }
}

void UpdateGame(void)
{
    UpdatePlayer();
    UpdateBullets();
    UpdateEnemies();
}

void DrawGame(void)
{
    DrawPlayer(&player);
    DrawEnemies();
    DrawBullets();

    DrawText(TextFormat("SCORE %llu", score), 0, 20, 20, WHITE);
    DrawText(TextFormat("ROTATION %.3f", player.rotation), 0, 40, 20, BLUE);
    DrawText(TextFormat("BULLETS %u", bulletCount), 0, 60, 20, BLUE);
    DrawText(TextFormat("ENEMIES %u", enemyCount), 0, 80, 20, BLUE);
}

void ResetGame(void)
{
    player.x = SCREEN_WIDTH/2.0f;
    player.y = SCREEN_HEIGHT/2.0f;
    player.isReloading = false;
    player.rotation = 0.0f;
    player.color = (Color)PLAYER_COLOR;
    playerIsDead = false;
    playerReloadTimer = 0.0f;
    score = 0;
    
    // NOT NECESSARY
    for(unsigned int i = 0;i < MAX_ENEMIES;i++)
    {
        if(enemyList[i] != 0)
        {
            free(enemyList[i]);
            enemyList[i] = 0;
        }
    }
    enemyCount = 0;
    enemyTimer = 0.0f;

    for(unsigned int i = 0;i < MAX_BULLETS;i++)
    {
        if(bulletList[i] != 0)
        {
            free(bulletList[i]);
            bulletList[i] = 0;
        }
    }
    bulletCount = 0;
}