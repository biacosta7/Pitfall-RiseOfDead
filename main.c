#include <raylib.h>
#include <stdio.h>
#include <math.h>

#define SCALE_FACTOR 1.6
#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 800

// struct player
typedef struct{
    Vector2 position;
    Vector2 size;
    Color color;
    float velocityY; // velocidade vertical (para controlar o pulo)
    bool isJumping;
} Player;

// struct enemy
typedef struct{
    Vector2 position;
    Vector2 size;
    Color color;
    float velocityY;
} Enemy;

// draw player e enemy
void DrawPlayer(Player player){
    DrawRectangleV(player.position, player.size, player.color);
}

void aplica_gravidade(Player *player, Enemy *enemy) {
    float gravidade = 0.5f;
    player->velocityY += gravidade; // acumula a gravidade na velocidade
    
    player->position.y += player->velocityY; // atualiza a posição com base na velocidade
    
    // verifica se o player atingiu o chão
    float groundLevel = (SCREEN_HEIGHT) - player->size.y;
    if (player->position.y > groundLevel) {
        player->position.y = groundLevel;
        player->velocityY = 0; // para a velocidade quando o player chega no chão
        player->isJumping = false; 
    }

    enemy->velocityY += gravidade;
    enemy->position.y += enemy->velocityY;

    if (enemy->position.y > groundLevel) {
        enemy->position.y = groundLevel;
        enemy->velocityY = 0;
    }
}


int main(void){
    // cria window
    int screenWidth = SCREEN_WIDTH * SCALE_FACTOR;
    int screenHeight = SCREEN_HEIGHT * SCALE_FACTOR;
    InitWindow(screenWidth, screenHeight, "Pitfall - Rise Of Dead");

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
        if (IsKeyPressed(KEY_W) && !player.isJumping) {
            player.velocityY = -10.0f; // velocidade de pulo inicial
            player.isJumping = true;
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

        aplica_gravidade(&player, &enemy);

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