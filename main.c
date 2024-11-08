#include <raylib.h>
#include <stdio.h>
#include <math.h>

#define SCALE_FACTOR 1.6
#define SCREEN_HEIGHT 900
#define SCREEN_WIDTH 40

// struct player
typedef struct{
    Vector2 position;
    Vector2 size;
    Color color;
} Player;

// struct enemy
typedef struct{
    Vector2 position;
    Vector2 size;
    Color color;
} Enemy;

// draw player e enemy
void DrawPlayer(Player player){
    DrawRectangleV(player.position, player.size, player.color);
}

int main(void){
    // cria window
    int screenWidth = 800 * SCALE_FACTOR;
    int screenHeight = 600 * SCALE_FACTOR;
    InitWindow(screenWidth, screenHeight, "Raylib Game");

    // cria player
    Player player = {0};
    player.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    player.size = (Vector2){20, 20};
    player.color = RED;
    
    // cria enemy
    Enemy enemy = {0};
    enemy.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    enemy.size = (Vector2){20, 20};
    enemy.color = BLUE;
    
    // game loop
    while (!WindowShouldClose())
    {
        // movimento player
        if (IsKeyDown(KEY_W)){
            player.position.y -= 1;
        }
        if (IsKeyDown(KEY_A)){
            player.position.x -= 1;
        }
        if (IsKeyDown(KEY_D)){
            player.position.x += 1;
        }

        // inimigo seguindo o player
        if (player.position.x > enemy.position.x){
            enemy.position.x += 0.3;
        } else if (player.position.x < enemy.position.x){
            enemy.position.x -= 0.3;
        }

        if (player.position.y > enemy.position.y){
            enemy.position.y += 0.3;
        } else if(player.position.y < enemy.position.y){
            enemy.position.y -= 0.3;
        }

        // draw the game
        BeginDrawing();
        ClearBackground(BLACK);
        DrawPlayer(player);
        DrawRectangleV(enemy.position, enemy.size, enemy.color);
        if (CheckCollisionRecs((Rectangle){player.position.x, player.position.y, player.size.x, player.size.y}, (Rectangle){enemy.position.x, enemy.position.y, enemy.size.x, enemy.size.y}))
        {
            DrawText("Game Over", screenWidth / 2 - 100, screenHeight / 2 - 50, 20, RED);
        }
        EndDrawing();
    }
    // fecha a janela
    CloseWindow();
    return 0;
}