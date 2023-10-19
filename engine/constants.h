#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 800
#define MATTEBLACK CLITERAL(Color){ 40, 40, 43, 255 }      // Matte Black
#define BACKGROUND_COLOR MATTEBLACK
#define UP (Vector2){0, -1}

#define RANDBETWEEN(A, B) ((rand() % (B - A + 1)) + A)
#define MAX(A, B) (A > B) ? A : B
#define MIN(A, B) (A < B) ? A : B

#define PLAYER_WIDTH 80
#define PLAYER_HEIGHT ((PLAYER_WIDTH*1.73205)/2.0f) // Equilateral triangle
#define PLAYER_MOVESPEED 360
#define PLAYER_ROTATESPEED 300.0f
#define PLAYER_RELOADSPEED 0.25f
#define PLAYER_COLOR { 255, 255, 255, 255 } // WHITE

#define MAX_ENEMIES 512
#define ENEMY_RADIUS 35.0f
#define ENEMY_SPEED 360
#define ENEMY_SPAWNTIME 0.25f
#define ENEMY_ROTATIONSPEED 2.0f
#define ENEMY_COLOR { 255, 161, 0, 255 } // ORANGE
#define ENEMY_SCORE 100

#define MAX_BULLETS 512
#define BULLET_RADIUS 6.0f
#define BULLET_SPEED (PLAYER_MOVESPEED*2)

#endif