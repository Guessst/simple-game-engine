#include "raylib.h"
#include "constants.h"
#include "game.h"

int main(void)
{
    // Initialization
    //----------------------------------------------------------------------------------------
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GAME 2 WITH ENGINE");

    SetTargetFPS(1000);

    // double finalTime;
    //----------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //------------------------------------------------------------------------------------
        UpdateGame();
        //------------------------------------------------------------------------------------

        // Draw
        //------------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);

            DrawGame();

            DrawFPS(0, 0);
        EndDrawing();
        //------------------------------------------------------------------------------------
    }

    // De-Initialization
    //----------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //----------------------------------------------------------------------------------------

    return 0;
}