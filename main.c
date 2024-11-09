#include <raylib.h>
#include <stdio.h>
#include <math.h>

#define SCALE_FACTOR 1.6
#define SCREEN_HEIGHT 600
#define SCREEN_WIDTH 800
#define LAYER_COUNT 3

typedef enum { IDLE, RUNNING, JUMPING } PersonagemState;

// struct player
typedef struct{
    Vector2 position;
    Vector2 size;
    float velocityY; // velocidade vertical (para controlar o pulo)
    bool isJumping;
    PersonagemState state;
    Texture2D idleTexture;
    Texture2D runTexture;
    Texture2D jumpTexture;
    int frame;           // Frame atual da animação
    float frameTime;     // Tempo entre frames
    float currentFrameTime; // Acumulador de tempo
    int maxFrames;       // Número de frames na spritesheet
    bool flipRight; // controla a direção
} Player;

// struct enemy
typedef struct{
    Vector2 position;
    Vector2 size;
    Color color;
    float velocityY;
    PersonagemState state;
    Texture2D idleTexture;
    Texture2D runTexture;
    int frame;           // Frame atual da animação
    float frameTime;     // Tempo entre frames
    float currentFrameTime; // Acumulador de tempo
    int maxFrames;       // Número de frames na spritesheet
    bool flipRight; // controla a direção
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

void UpdatePlayerAnimation(Player *player, float deltaTime) {
    player->currentFrameTime += deltaTime;

    // avança para o próximo frame se o tempo decorrido for suficiente
    if (player->currentFrameTime >= player->frameTime) {
        player->currentFrameTime = 0;
        player->frame++;
        
        // volta para o primeiro frame ao atingir o último
        if (player->frame >= player->maxFrames) {
            player->frame = 0;
        }
    }
}

void DrawPlayer(Player player) {
    Texture2D texture;
    
    switch (player.state) {
        case RUNNING:
            texture = player.runTexture;
            break;
        case JUMPING:
            texture = player.jumpTexture;
            break;
        case IDLE:
        default:
            texture = player.idleTexture;
            break;
    }
    
    if (player.maxFrames <= 0) {
        player.maxFrames = 1;  // Set a safe default
    }
    int frameWidth = texture.width / player.maxFrames;
    
    // define a área do frame atual na spritesheet
    Rectangle sourceRect = { frameWidth * player.frame, 0, frameWidth, texture.height };

    // inverte o `sourceRect.width` se `flipRight` for false
    if (!player.flipRight) {
        sourceRect.width = -frameWidth;
    }

    Rectangle destRect = { player.position.x, player.position.y, player.size.x * 6, player.size.y * 6 };
    Vector2 origin = { 0, 0 };
    
    // desenha o frame atual da spritesheet, aplicando o flip horizontal se necessário
    DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
}

void UpdateEnemyAnimation(Enemy *enemy, float deltaTime) {
    enemy->currentFrameTime += deltaTime;

    // avança para o próximo frame se o tempo decorrido for suficiente
    if (enemy->currentFrameTime >= enemy->frameTime) {
        enemy->currentFrameTime = 0;
        enemy->frame++;
        
        // volta para o primeiro frame ao atingir o último
        if (enemy->frame >= enemy->maxFrames) {
            enemy->frame = 0;
        }
    }
}

void DrawEnemy(Enemy enemy) {
    Texture2D texture;
    
    switch (enemy.state) {
        case RUNNING:
            texture = enemy.runTexture;
            break;
 
        case IDLE:
        default:
            texture = enemy.idleTexture;
            break;
    }

    if (enemy.maxFrames <= 0) {
        enemy.maxFrames = 1;  // Set a safe default
    }
    int frameWidth = texture.width / enemy.maxFrames;
        
    // define a área do frame atual na spritesheet
    Rectangle sourceRect = { frameWidth * enemy.frame, 0, frameWidth, texture.height };

    // inverte o `sourceRect.width` se `flipRight` for false
    if (!enemy.flipRight) {
        sourceRect.width = -frameWidth;
    }

    Rectangle destRect = { enemy.position.x, enemy.position.y, enemy.size.x * 6, enemy.size.y * 6 };
    Vector2 origin = { 0, 0 };
    
    // desenha o frame atual da spritesheet, aplicando o flip horizontal se necessário
    DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
}


void aplica_gravidade(Player *player, Enemy *enemy) {
    float gravidade = 0.5f;
    player->velocityY += gravidade; // acumula a gravidade na velocidade
    
    player->position.y += player->velocityY; // atualiza a posição com base na velocidade
    
    // verifica se o player atingiu o chão
    float groundLevel = (SCREEN_HEIGHT) - player->size.y * 5;
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

void limitar_player(Player *player, int screenWidth, int screenHeight) {
    if (player->position.x < 0) {
        player->position.x = 0;
    }
    if (player->position.x + player->size.x > screenWidth) {
        player->position.x = screenWidth - player->size.x;
    }
    if (player->position.y < 0) {
        player->position.y = 0;
    }
    if (player->position.y + player->size.y > screenHeight) {
        player->position.y = screenHeight - player->size.y;
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
    player.position = (Vector2){(SCREEN_WIDTH / 2), SCREEN_HEIGHT / 2};
    player.size = (Vector2){64, 64};
    player.state = IDLE;
    player.idleTexture = LoadTexture("assets/player/idle2.png");
    player.runTexture = LoadTexture("assets/player/run.png");
    player.jumpTexture = LoadTexture("assets/player/jump.png");
    player.maxFrames = 5; // quantidade de frames de IDLE
    player.frame = 0;
    player.frameTime = 0.3f; // Tempo entre frames
    player.currentFrameTime = 0.0f;
    player.flipRight = true; 
    player.velocityY = 0;    
    
    // cria enemy
    Enemy enemy = {0};
    enemy.position = (Vector2){0, screenHeight / 2};
    enemy.size = (Vector2){64, 64};
    enemy.state = IDLE;
    enemy.idleTexture = LoadTexture("assets/zombie/Idle.png");
    enemy.runTexture = LoadTexture("assets/zombie/run.png");
    enemy.maxFrames = 7;
    enemy.frame = 0;
    enemy.frameTime = 0.3f;
    enemy.currentFrameTime = 0.0f;
    enemy.flipRight = true;
    enemy.velocityY = 0;
    
    // game loop
    while (!WindowShouldClose()){

        bool colidiu = CheckCollisionRecs((Rectangle){player.position.x, player.position.y, player.size.x, player.size.y}, (Rectangle){enemy.position.x, enemy.position.y, (enemy.size.x)+50, enemy.size.y});

        bool movingHorizontal = false;
        bool movingLeft = false;

        // movimento player
        if (IsKeyPressed(KEY_W) && !player.isJumping) {
            player.velocityY = -10.0f; // velocidade de pulo inicial
            player.isJumping = true;
            player.state = JUMPING;
            player.frame = 0; // resetar para o primeiro frame de pulo
            player.maxFrames = 8;
            player.frameTime = 0.2f;
        }
        if (IsKeyDown(KEY_A)){
            player.position.x -= 1;
            movingHorizontal = true;
            movingLeft = true;
            player.flipRight = false; // Vira para a esquerda

            if (player.state != RUNNING) {
                player.state = RUNNING;
                player.frame = 0; // Reseta o frame ao entrar no estado RUNNING
                player.maxFrames = 8;
                player.frameTime = 0.1f;
            }

        }
        if (IsKeyDown(KEY_D)){
            player.position.x += 1;
            movingHorizontal = true;
            movingLeft = false;
            player.flipRight = true; // Vira para a direita

            if (player.state != RUNNING) {
                player.state = RUNNING;
                player.frame = 0;
                player.maxFrames = 8;
                player.frameTime = 0.1f;
            }
        }
        else if (!movingHorizontal && !player.isJumping) {
            if (player.state != IDLE) {
                player.state = IDLE;
                player.frame = 0;
                player.maxFrames = 5;
                player.frameTime = 0.3f;
            }
        }

        limitar_player(&player, screenWidth, screenHeight);

        // inimigo seguindo o player
        if (player.position.x > enemy.position.x){
            enemy.position.x += 0.5;
            if (enemy.state != RUNNING) {
                enemy.state = RUNNING;
                enemy.frame = 0;
                enemy.maxFrames = 7;
                enemy.frameTime = 0.1f;
            }
        } else if (player.position.x < enemy.position.x){
            enemy.position.x -= 0.5;
            if (enemy.state != RUNNING) {
                enemy.state = RUNNING;
                enemy.frame = 0;
                enemy.maxFrames = 7;
                enemy.frameTime = 0.1f;
            }
        } else if(colidiu){
            if (enemy.state != IDLE) {
                enemy.state = IDLE;
                enemy.frame = 0;
                enemy.maxFrames = 8;
                enemy.frameTime = 0.3f;
            }
        } else{
            if (enemy.state != IDLE) {
                enemy.state = IDLE;
                enemy.frame = 0;
                enemy.maxFrames = 8;
                enemy.frameTime = 0.3f;
            }
        } 

        //zumbi nao pula
        // if (player.position.y > enemy.position.y){
        //     enemy.position.y += 0.3;
        // } else if(player.position.y < enemy.position.y){
        //     enemy.position.y -= 0.3;
        // }

        aplica_gravidade(&player, &enemy);
        
        float deltaTime = GetFrameTime();
        UpdatePlayerAnimation(&player, deltaTime);
        UpdateEnemyAnimation(&enemy, deltaTime);

        // draw the game
        BeginDrawing();
        ClearBackground(RAYWHITE);
        UpdateDrawParallax(layers, LAYER_COUNT, GetFrameTime(), screenWidth, screenHeight, movingHorizontal, movingLeft);
        DrawEnemy(enemy);
        DrawPlayer(player);

        if (colidiu){
            DrawText("Game Over", screenWidth / 2 - 100, screenHeight / 2 - 50, 20, RED);
        }
        EndDrawing();
    }
    // unload texturas
    UnloadTexture(player.idleTexture);
    UnloadTexture(player.runTexture);
    UnloadTexture(player.jumpTexture);
    UnloadTexture(enemy.idleTexture);
    UnloadTexture(enemy.runTexture);

    for (int i = 0; i < LAYER_COUNT; i++) {
        UnloadTexture(layers[i].texture);
    }
    // fecha a janela
    CloseWindow();
    return 0;
}