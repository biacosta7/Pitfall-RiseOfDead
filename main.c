#include <raylib.h>
#include <stdio.h>
#include <math.h>

#define SCALE_FACTOR 1.6
#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 800
#define LAYER_COUNT 3

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

typedef struct {
    Texture2D texture;
    Vector2 position;
    float speed;
} BackgroundLayer;

void DrawBackground(Texture2D background, int screenWidth, int screenHeight) {
    float scale;
    float offsetX = 0;
    float offsetY = 0;

    // Calculate scaling factor to cover the entire screen
    float scaleX = (float)screenWidth / background.width;
    float scaleY = (float)screenHeight / background.height;
    
    // Use the larger scaling factor to ensure the image covers the entire screen
    scale = (scaleX > scaleY) ? scaleX : scaleY;

    // Calculate offsets to center the image if it's larger than the screen
    if (background.width * scale > screenWidth)
        offsetX = (screenWidth - (background.width * scale)) / 2;
    if (background.height * scale > screenHeight)
        offsetY = (screenHeight - (background.height * scale)) / 2;

    // Draw the scaled background
    DrawTextureEx(background, (Vector2){offsetX, offsetY}, 0, scale, WHITE);
}

// atualiza o parallax
void UpdateDrawParallax(BackgroundLayer *layers, int layerCount, float deltaTime, int screenWidth, int screenHeight, bool movingHorizontal, bool movingLeft) {
    for (int i = 0; i < layerCount; i++) {
        // escala com base na altura da tela
        float scale = (float)screenHeight / layers[i].texture.height;

        // atualiza a posição se o player estiver se movendo
        if (movingHorizontal) {
            if (movingLeft) {
                layers[i].position.x += layers[i].speed * deltaTime;
            } else {
                layers[i].position.x -= layers[i].speed * deltaTime;
            }
        }

        // reseta a posição se a textura ultrapassar a tela
        if (layers[i].position.x <= -(layers[i].texture.width * scale)) {
            layers[i].position.x = 0;
        }

        // centraliza o fundo horizontalmente se for menor que a largura da tela
        float offsetX = 0;
        if (layers[i].texture.width * scale < screenWidth) {
            offsetX = (screenWidth - (layers[i].texture.width * scale)) / 2;
        }

        // desenha a textura duas vezes para criar um scroll contínuo
        DrawTextureEx(layers[i].texture, (Vector2){layers[i].position.x + offsetX, 0}, 0, scale, WHITE);
        DrawTextureEx(layers[i].texture, (Vector2){layers[i].position.x + (layers[i].texture.width * scale) + offsetX, 0}, 0, scale, WHITE);
    }
}

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

    BackgroundLayer layers[LAYER_COUNT] = {
        { LoadTexture("assets/map/layers/bg1.png"), (Vector2){0, 0}, 50 },
        { LoadTexture("assets/map/layers/bg3.png"), (Vector2){0, 0}, 100 },
        { LoadTexture("assets/map/layers/ground2.png"), (Vector2){0, 0}, 200 }
    };

    SetTargetFPS(60);

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
    while (!WindowShouldClose()){

        bool movingHorizontal = false;
        bool movingLeft = false;

        // movimento player
        if (IsKeyPressed(KEY_W) && !player.isJumping) {
            player.velocityY = -10.0f; // velocidade de pulo inicial
            player.isJumping = true;
        }
        if (IsKeyDown(KEY_A)){
            player.position.x -= 1;
            movingHorizontal = true;
            movingLeft = true;
        }
        if (IsKeyDown(KEY_D)){
            player.position.x += 1;
            movingHorizontal = true;
            movingLeft = false;
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
        
        float deltaTime = GetFrameTime();

        // draw the game
        BeginDrawing();
        ClearBackground(RAYWHITE);
        UpdateDrawParallax(layers, LAYER_COUNT, deltaTime, screenWidth, screenHeight, movingHorizontal, movingLeft);
        DrawPlayer(player);
        DrawRectangleV(enemy.position, enemy.size, enemy.color);
        
        if (CheckCollisionRecs((Rectangle){player.position.x, player.position.y, player.size.x, player.size.y}, (Rectangle){enemy.position.x, enemy.position.y, enemy.size.x, enemy.size.y})){
            DrawText("Game Over", screenWidth / 2 - 100, screenHeight / 2 - 50, 20, RED);
        }
        EndDrawing();
    }
    // unload texturas
    for (int i = 0; i < LAYER_COUNT; i++) {
        UnloadTexture(layers[i].texture);
    }
    // fecha a janela
    CloseWindow();
    return 0;
}