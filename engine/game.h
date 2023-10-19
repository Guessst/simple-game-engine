#ifndef GAME_H_
#define GAME_H_

#include "raylib.h"
#include "constants.h"

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

void MovePlayerWithKeyboard(void);
void MovePlayerWithGamepad(void);
void RotatePlayerWithKeyboard(void);
void RotatePlayerWithGamepad(void);
void Shoot(void);
void UpdatePlayer(void);
void AddEnemy(Enemy* enemy);
void SpawnEnemy(void);
void UpdateEnemies(void);
void DrawEnemies(void);
Bullet* InitBullet(float posX, float posY, Vector2 direction);
void AddBullet(Bullet* bullet);
void UpdateBullets(void);
void DrawBullets(void);
void UpdateGame(void);
void DrawGame(void);
void ResetGame(void);

#endif