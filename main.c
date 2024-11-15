#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define SCALE_FACTOR 1.6
#define SCREEN_HEIGHT 450
#define SCREEN_WIDTH 800
#define LAYER_COUNT 2
#define MAX_LIVES 3
#define GRAVITY 15.0f
#define MAX_ENEMIES 5

typedef enum { IDLE, RUNNING, JUMPING, ATTACK } PersonagemState;
typedef enum { START_SCREEN, GAMEPLAY } GameState;
const char *historiaDoJogo = "Em uma sociedade marcada pela decadência, a elite recrutou\n\n" 
                                "uma equipe de cientistas e após anos de pesquisa em um projeto\n\n"
                                "secreto, criou uma arma biológica destinada à imortalidade,\n\n"
                                "acreditando ser a única esperança para a sobrevivência\n\n"
                                "humana. Porém, o experimento saiu do controle, transformando \n\n"
                                "a maioria da população em zumbis. Você é um dos poucos que \n\n"
                                "escaparam de uma tentativa de invasão a essa fortaleza,\n\n"
                                "mas agora, na floresta densa, hordas de zumbis estão por\n\n"
                                "toda parte. Para alcançar a segurança, você deve correr\n\n"
                                "por perigosas áreas infestadas, coletar suprimentos.\n\n"
                                "Sua meta é chegar a um abrigo subterrâneo, onde os últimos\n\n"
                                "cientistas tentam criar uma vacina.\n\n";
const char *tituloDoJogo = "Pitfall: Rise of Dead";

// struct player
typedef struct{
    int x;
    int y;
    int width;
    int height;
    float velocityY; // velocidade vertical (para controlar o pulo)
    bool isJumping;
    bool canJump;
    PersonagemState state;
    Texture2D idleTexture;
    Texture2D runTexture;
    Texture2D jumpTexture;
    Texture2D attackTexture;
    int frame;           // Frame atual da animação
    float frameTime;     // Tempo entre frames
    float currentFrameTime; // Acumulador de tempo
    int maxFrames;       // Número de frames na spritesheet
    bool flipRight; // controla a direção
    int lives;
    Texture2D heartTexture3;
    Texture2D heartTexture2;
    Texture2D heartTexture1;
    bool invencivel; //tempo de invencibilidade logo apos levar uma colisao
    float invencibilidadeTimer; // tempo que dura a invencibilidade
} Player;


// struct enemy
typedef struct{
    int x;
    int y;
    int width;
    int height;
    float velocityY;
    bool isAttacking;
    PersonagemState state;
    Texture2D idleTexture;
    Texture2D runTexture;
    Texture2D attackTexture;
    int frame;           // Frame atual da animação
    float frameTime;     // Tempo entre frames
    float currentFrameTime; // Acumulador de tempo
    int maxFrames;       // Número de frames na spritesheet
    bool flipRight; // controla a direção
    int enemyLives;
} Enemy;

typedef struct {
    Enemy enemy;
    bool isActive;
    float spawnX;
} EnemySpawner;

typedef struct {
    Texture2D texture;
    Vector2 position;
    float speed;
} BackgroundLayer;

typedef enum {
    PLATFORM,
    FLOOR,
} PlatformType;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    PlatformType type;
    Texture2D texture;
} Platform;

// struct Winners{
//     char nome[20];
//     int tempo;
//     struct Winners *next;
// };
// struct Winners *head = NULL;
// void add_winner(struct Winners **head, char *nome, int tempo){
//     struct Winners *n = *head;
//     struct Winners *novo = (struct Winners *)malloc(sizeof(struct Winners));
//     struct Winners *anterior = NULL;

//     strcpy(novo->nome, nome);
//     novo->tempo = tempo;
//     novo->next = NULL;

//     if(*head == NULL){
//         *head = novo;
//         return;
//     }

//     if((*head)->tempo > novo->tempo){
//         novo->next = *head;
//         *head = novo;
//         return;
//     }
//     while(n != NULL && n->tempo <= novo -> tempo){
//         anterior = n;
//         n = n-> next;
//     }
//     if(anterior != NULL){
//         anterior -> next = novo;
//     }
//     novo -> next = n;
//     return;
// }
// void winnerList(){
//     FILE *list;
//     char nome[20];
//     int tempo;
//     list = fopen("vencedores.txt", "r");
//     while(fscanf(list, "%s %d", nome, &tempo) == 2){
//         add_winner(&head, nome, tempo);
//     }
//     fclose(list);
// }
// void printwinnerList(struct Winners *head){
//     struct Winners *n = *head;
//     int i = 1;
//     while(n != NULL && i <= 10){
//         printf("%d. %s: %d segundos", i, n->nome, n->tempo);
//         n = n->next;
//         i++;
//     }
//     printf("\n");
// }
// void writeWinners(){
//     FILE *list;
//     list = fopen("winners.txt", "w");
//     struct Winners *n = head;
//     while(n != NULL){
//         fprintf(list,"%s %d\n", n->nome, n->tempo);
//         n=n->next;
//     }
//     fclose(list);
// }

void aplica_gravidade_player(Player *player, Platform platforms[], int total_platform_count, float deltaTime);
void aplica_gravidade_enemy(Enemy *enemy, Platform platforms[], int total_platform_count, float deltaTime);

void DrawBackground(Texture2D background, int screenWidth, int screenHeight, Camera2D camera) {
    float scale;
    float deslocamento_x = 0;
    float deslocamento_y = 0;

    // Calcular fator de escala para cobrir a tela inteira
    float scaleX = (float)screenWidth / background.width;
    float scaleY = (float)screenHeight / background.height;
    
    // Escolher o maior fator de escala para garantir que a imagem cubra a tela
    if (scaleX > scaleY) {
        scale = scaleX;
    } else {
        scale = scaleY;
    }

    // Ajustar deslocamento para centralizar a imagem
    if (background.width * scale > screenWidth)
        deslocamento_x = (screenWidth - (background.width * scale)) / 2;
    if (background.height * scale > screenHeight)
        deslocamento_y = (screenHeight - (background.height * scale)) / 2;

    // Aplicar o deslocamento da câmera para o efeito parallax
    deslocamento_x += camera.offset.x * 0.1f; 
    deslocamento_y += camera.offset.y * 0.1f;

    // Desenhar o background escalado
    DrawTextureEx(background, (Vector2){deslocamento_x, deslocamento_y}, 0, scale, WHITE);
}

void DrawLives(Player player, Camera2D camera) {
    // Define a posição inicial para desenhar as vidas, ajustada pela posição da câmera
    int posX = 20 - camera.offset.x;
    int posY = 20 - camera.offset.y;

    // Desenha o ícone de vida correto baseado na quantidade de vidas do jogador
    if (player.lives == 3) {
        DrawTexture(player.heartTexture3, posX, posY, WHITE);
    } else if (player.lives == 2) {
        DrawTexture(player.heartTexture2, posX, posY, WHITE);
    } else if (player.lives == 1) {
        DrawTexture(player.heartTexture1, posX, posY, WHITE);
    }
}
void DrawTimer(Camera2D camera, int timerValue) {
    // Define a posição do timer ajustada pela câmera
    int posX = 20 - camera.offset.x;
    int posY = 20 - camera.offset.y;

    // Converte o valor do timer para string
    char timerText[10];
    snprintf(timerText, sizeof(timerText), "%d", timerValue);

    // Desenha o texto na posição ajustada
    DrawText(timerText, posX, posY, 20, RED);
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
        case ATTACK:
            texture = player.attackTexture;
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

    Rectangle destRect = { player.x, player.y, player.width * 6, player.height * 6 };
    Vector2 origin = { 0, 0 };
    
    // desenha o frame atual da spritesheet, aplicando o flip horizontal se necessário
    DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
}

void UpdateEnemyAnimation(Enemy *enemy, float deltaTime) {
    enemy->currentFrameTime += deltaTime;

    // Advance to next frame if enough time has passed
    if (enemy->currentFrameTime >= enemy->frameTime) {
        enemy->currentFrameTime = 0;
        enemy->frame++;
        
        // Reset frame based on current state
        switch(enemy->state) {
            case RUNNING:
                if (enemy->frame >= 7) {
                    enemy->frame = 0;
                    enemy->frameTime = 0.5f;
                    break;
                }
                
            case ATTACK:
                if (enemy->frame >= 4) {
                    enemy->frame = 0;
                    // Optionally return to IDLE after attack animation
                    enemy->state = IDLE;
                    enemy->maxFrames = 8;
                    enemy->frameTime = 1.0f;
                }
                break;
                
            case IDLE:
                if (enemy->frame >= 8) enemy->frame = 0;
                break;
                
            default:
                if (enemy->frame >= enemy->maxFrames) enemy->frame = 0;
                break;
        }
    }
}

void DrawEnemy(Enemy enemy) {
    Texture2D texture;
    
    switch (enemy.state) {
        case RUNNING:
            texture = enemy.runTexture;
            break;
        case ATTACK:
            texture = enemy.attackTexture;
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

    Rectangle destRect = { enemy.x, enemy.y, enemy.width * 6, enemy.height * 6 };
    Vector2 origin = { 0, 0 };
    
    // desenha o frame atual da spritesheet, aplicando o flip horizontal se necessário
    DrawTexturePro(texture, sourceRect, destRect, origin, 0.0f, WHITE);
}

void InitEnemySpawners(EnemySpawner enemies[], int count, Enemy baseEnemy) {
    float spawnDistance = 800.0f;  // Distance between spawn points
    
    for(int i = 0; i < count; i++) {
        enemies[i].enemy = baseEnemy;  // Copy the base enemy properties
        enemies[i].isActive = false;
        enemies[i].spawnX = (i + 1) * spawnDistance;  // Set spawn points progressively further
    }
}

int player_na_plataforma(Player player, Platform platforms[], int total_platform_count) {
    Rectangle player_rec = {
        .x = player.x + (player.width * 2),
        .y = player.y + (player.height * 6) - 10,
        .width = player.width,
        .height = 10  // Small height for ground detection
    };

    for(int i = 0; i < total_platform_count; i++) {
        Rectangle platform_rec = {
            .x = platforms[i].x,
            .y = platforms[i].y,
            .width = platforms[i].width,
            .height = platforms[i].height
        };

        if(CheckCollisionRecs(player_rec, platform_rec)) {
            if (platforms[i].type == FLOOR){
                return i;
            } else{
                return -1;
            } 
        }
        
    }
    return -1;
}

int enemy_na_plataforma(Enemy enemy, Platform platforms[], int total_platform_count) {
    Rectangle enemy_rec = {
        .x = enemy.x + (enemy.width * 2),
        .y = enemy.y + (enemy.height * 6) - 10,
        .width = enemy.width * 2,
        .height = 10 
    };

    for(int i = 0; i < total_platform_count; i++) {
        Rectangle platform_rec = {
            .x = platforms[i].x,
            .y = platforms[i].y,
            .width = platforms[i].width,
            .height = platforms[i].height
        };

        if(CheckCollisionRecs(enemy_rec, platform_rec)) {
            if (platforms[i].type == FLOOR){
                return i;
            } else{
                return -1;
            } 
        }
        
    }
    return -1;
}

bool enemy_colide_player(Enemy enemy, Player player){
    Rectangle enemy_rec = {
        .x = enemy.x + (enemy.width * 2),
        .y = enemy.y,
        .width = enemy.width * 3,
        .height = 10 
    };

    Rectangle player_rec = {
        .x = player.x + (player.width * 2),
        .y = player.y,
        .width = player.width * 3,
        .height = 10 
    };
    
    printf("Enemy: x=%f, y=%f, Player: x=%f, y=%f\n", 
           enemy_rec.x, enemy_rec.y, player_rec.x, player_rec.y);

    return CheckCollisionRecs(enemy_rec, player_rec);
    
}

void UpdateEnemyPosition(Enemy *enemy, Player player) {
    bool is_colliding = enemy_colide_player(*enemy, player);

    if (is_colliding){
        enemy->isAttacking = true;
        if (enemy->state != ATTACK) {
            enemy->state = ATTACK;
            enemy->frame = 0;
            enemy->maxFrames = 4;
            enemy->frameTime = 0.2f;
        }
    }
    
    else if (player.x > enemy->x && !is_colliding){
        enemy->x += 2.0f;
        enemy->flipRight = true;
        enemy->isAttacking = false;
        if (enemy->state != RUNNING) {
            enemy->state = RUNNING;
            enemy->frame = 0;
            enemy->maxFrames = 7;
            enemy->frameTime = 0.3f;
        }
    } else if (player.x < enemy->x && !is_colliding){
        enemy->x -= 2.0f;
        enemy->flipRight = false;
        enemy->isAttacking = false;
        if (enemy->state != RUNNING) {
            enemy->state = RUNNING;
            enemy->frame = 0;
            enemy->maxFrames = 7;
            enemy->frameTime = 0.3f;
        }
    } 
    else {
        enemy->isAttacking = false;
        if (enemy->state != IDLE) {
            enemy->state = IDLE;
            enemy->frame = 0;
            enemy->maxFrames = 8;
            enemy->frameTime = 0.3f;
        }
    }
}

// Update and manage enemy spawning
void UpdateEnemies(EnemySpawner enemies[], int count, Player player, Platform *platforms, int total_platform_count) {
    float spawnTriggerDistance = 400.0f;  // Distance before spawn point when enemy should appear
    
    for(int i = 0; i < count; i++) {
        if (!enemies[i].isActive) {
            // Check if player is approaching this spawn point
            if (player.x > (enemies[i].spawnX - spawnTriggerDistance)) {
                enemies[i].isActive = true;
                enemies[i].enemy.x = enemies[i].spawnX;
                enemies[i].enemy.y = player.y;  // Or your ground level
            }
        }
        else {
            aplica_gravidade_enemy(&enemies[i].enemy, platforms, total_platform_count, GetFrameTime());
            UpdateEnemyPosition(&enemies[i].enemy, player); // Update active enemy
            UpdateEnemyAnimation(&enemies[i].enemy, GetFrameTime()); // Update enemy animation
        }
    }
}

void aplica_gravidade_player(Player *player, Platform platforms[], int total_platform_count, float deltaTime) {
    const float MAX_FALL_SPEED = 10.0f;  // Maximum falling speed
    
    // Apply gravity with deltaTime only if jumping or in air
    if (player->isJumping || player->velocityY != 0) {
        player->velocityY += GRAVITY * deltaTime;
    }

    // Clamp fall speed
    if (player->velocityY > MAX_FALL_SPEED) {
        player->velocityY = MAX_FALL_SPEED;
    }
    
    // Update position
    player->y += player->velocityY * deltaTime * 60.0f; // Normalize for 60 FPS
    
    // Check platform collision
    int current_platform = player_na_plataforma(*player, platforms, total_platform_count);

    if (current_platform != -1) {
        // If colliding with platform
        if (player->velocityY > 0) {  // Only if moving downward
            // Snap to platform top
            player->isJumping = false;
            player->canJump = true;    // Allow jumping when on platform
            player->y = platforms[current_platform].y - (player->height * 6);
            player->velocityY = 0.0;
        }
    } else {
        // If in air
        player->canJump = false;       // Can't jump while in air
        if (player->velocityY != 0) {
            player->isJumping = true;
        }
    }
}

void aplica_gravidade_enemy(Enemy *enemy, Platform platforms[], int total_platform_count, float deltaTime) {
    const float MAX_FALL_SPEED = 10.0f;  // Maximum falling speed
    
    // Apply gravity with deltaTime
    enemy->velocityY += GRAVITY * deltaTime;

    // Clamp fall speed
    if (enemy->velocityY > MAX_FALL_SPEED) {
        enemy->velocityY = MAX_FALL_SPEED;
    }
    
    // Update position
    enemy->y += enemy->velocityY * deltaTime * 60.0f;
    
    // Check platform collision
    int current_platform_enemy = enemy_na_plataforma(*enemy, platforms, total_platform_count);

    if (current_platform_enemy != -1) {
        // If colliding with platform
        if (enemy->velocityY > 0) {  // Only if moving downward
            // Snap to platform top
            enemy->y = platforms[current_platform_enemy].y - (enemy->height * 6);
            enemy->velocityY = 0;
        }
    }
}


int main(void){
    bool isGameOver = false;
    // cria window
    int screenWidth = SCREEN_WIDTH * SCALE_FACTOR;
    int screenHeight = SCREEN_HEIGHT * SCALE_FACTOR;
    InitWindow(screenWidth, screenHeight, "Pitfall - Rise Of Dead");
    double startTime = 0.0;
    bool timeStarted = false;
    Texture2D backgroundTitle = LoadTexture("assets/map/layers/initialbackground.png");
    Texture2D floor_piece_texture = LoadTexture( "assets/map/floor.png");
    Font fontePersonalizada = LoadFont("assets/fonts/bloodcrow.ttf");
    Texture2D platform1_texture = LoadTexture("assets/map/platform-floor.png");
    Texture2D platform2_texture = LoadTexture("assets/obstaculos/platform2.png");
    Texture2D background_texture = LoadTexture( "assets/map/layers/bg1.png" );
    Texture2D background2_texture = LoadTexture( "assets/map/layers/bg2.png" );
    SetTargetFPS(60);
    GameState gameState = START_SCREEN;

    Vector2 camera_deslocamento = {
        .x = 0,
        .y = 0,
    };

    Vector2 camera_alvo = {
        .x = 0,
        .y = 0,
    };

    Camera2D camera = {
        .offset = camera_deslocamento,
        .target = camera_alvo,
        .rotation = 0,
        .zoom = 1,
    };

    int worldWidth = screenWidth * 10;

    int background_width = 1820;
    int background_overflow = background_width - screenWidth;
    float background_ratio = 1 / ((float)(worldWidth - screenWidth) / (float)background_overflow);
    float background2_ratio = 1.5f * (1 / ((float)(worldWidth - screenWidth) / (float)background_overflow)); // se move mais rápido
    int background_x = 0;
    int background2_x = 0;

    // definicões da plataforma
    int whitespace = 30; // espaco em branco da imagem, essa "margem/padding" do topo

    int platform_height = 190; // altura do chão
    int platform_width = 200; // tamanho (largura) de cada plataforma

    int total_platform_count = ceil((float)worldWidth / (float)platform_width); // calcula quantos pedacos de chão são necessários pra cobrir toda a largura do mundo
    
    // criando chão (floor)
    Platform platforms[total_platform_count];
    
    // Alternância das plataformas sem espaços entre elas
    int platform_x = 0;  // acumula 

    for(int i=0; i < total_platform_count; i++){
        platforms[i].width = platform_width;
        platforms[i].height = platform_height;
        platforms[i].y = screenHeight - platform_height + whitespace;
        platforms[i].x = platform_x;
        if(i%7 == 6){ 
            platforms[i].type = PLATFORM; // Plataforma
        } else {
            platforms[i].type = FLOOR; // Floor
        }
        platform_x += platform_width;
    }

    // cria player
    Player player = {
        .x = SCREEN_WIDTH / 2,
        .y = 175,
        .width = 64,
        .height = 64,
        .state = IDLE,
        .isJumping = false,
        .canJump = true,
        .idleTexture = LoadTexture("assets/player/idle2.png"),
        .runTexture = LoadTexture("assets/player/run.png"),
        .jumpTexture = LoadTexture("assets/player/jump.png"),
        .attackTexture = LoadTexture("assets/player/attack.png"),
        .maxFrames = 5, // quantidade de frames de IDLE
        .frame = 0,
        .frameTime = 0.3f, // Tempo entre frames
        .currentFrameTime = 0.0f,
        .flipRight = true, 
        .velocityY = 0,  
        .lives = MAX_LIVES,  
        .heartTexture3 = LoadTexture("assets/Heart/Heart3.png"),
        .heartTexture2 = LoadTexture("assets/Heart/Heart2.png"),
        .heartTexture1 = LoadTexture("assets/Heart/Heart1.png"),
    };
    
    // cria enemy
    Enemy enemy = {
        .x = SCREEN_WIDTH / 2,
        .y = 150,
        .width = 64,
        .height = 64,
        .state = IDLE,
        .isAttacking = false,
        .idleTexture = LoadTexture("assets/zombie/Idle.png"),
        .runTexture = LoadTexture("assets/zombie/run.png"),
        .attackTexture = LoadTexture("assets/zombie/attack.png"),
        .maxFrames = 8,
        .frame = 0,
        .frameTime = 0.3f,
        .currentFrameTime = 0.0f,
        .flipRight = true,
        .velocityY = 0,
    };

    EnemySpawner enemies[MAX_ENEMIES];
    InitEnemySpawners(enemies, MAX_ENEMIES, enemy);
    
    // game loop
    while (!WindowShouldClose()){
        bool colidiu = false;
        bool movingHorizontal = false;
        bool movingLeft = false;
        float deltaTime = GetFrameTime();

        // draw the game
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if(gameState == START_SCREEN){
            BeginDrawing();
            if(IsKeyPressed(KEY_ENTER)){
                gameState = GAMEPLAY;
                startTime = GetTime(); //momento do começo do jogo
                timeStarted = true;
                isGameOver = false;
            }
            int posXtitulo = 420;
            int postYtitulo = 40;
            int posXhistoria = 155;
            int posYhistoria = 150;
            int fonttitulo = 40;
            int fonthistoria = 30;
            // cor titulo = preto, cor historia = branco, cor comando = vermelho
            Vector2 posTitulo = { 420, 40 };
            Vector2 posHistoria = { 155, 150 };

            DrawBackground(backgroundTitle, screenWidth, screenHeight, camera);
            
            DrawTextEx(fontePersonalizada, tituloDoJogo, posTitulo, 40, 1, BLACK); //posicao X, posicao Y, tamanho fonte, cor
            DrawTextEx(fontePersonalizada, historiaDoJogo, posHistoria, 30, 1, WHITE);
            DrawText("Pressione ENTER para iniciar a corrida!", 325, 600, 30, RED);
        }
        else if(gameState == GAMEPLAY){
            double elapsedTime;
            if (!timeStarted) {
                startTime = GetTime(); // Garante que o tempo de início é capturado apenas uma vez
                timeStarted = true;
            }
            if (!isGameOver) {
                double currentTime = GetTime();
                elapsedTime = currentTime - startTime; // Atualiza o tempo decorrido
            }
            // camera 2D
            BeginMode2D( camera );
            if( player.x > screenWidth * 0.1 ) {
                camera.offset.x = -(player.x - screenWidth * 0.1);

            } else if( player.x < screenWidth * 0.05 ) {
                camera.offset.x = -(player.x - screenWidth * 0.05);
            }

            if( camera.offset.x > 0 ) {
                camera.offset.x = 0;
            }

            if( camera.offset.x < -(worldWidth - screenWidth) ) {
                camera.offset.x = -(worldWidth - screenWidth);
            }

            background_x = -camera.offset.x;
            background_x -= background_x * background_ratio;

            background2_x = -camera.offset.x;
            background2_x -= background2_x * background2_ratio;

            const float MOVE_SPEED = 5.0f; // constante velocidade player

            // movimento player
            if (IsKeyPressed(KEY_W) && !player.isJumping) {
                player.velocityY = -400.0f * GetFrameTime();
                player.isJumping = true;
                player.state = JUMPING;
                player.frame = 0;
                player.maxFrames = 8;
                player.frameTime = 0.2f;
            }

            // Handle horizontal movement
            if (IsKeyDown(KEY_A)) {
                player.x -= MOVE_SPEED;
                movingHorizontal = true;
                movingLeft = true;
                player.flipRight = false;
                player.isJumping = false;
                
                if (player.state != RUNNING) {
                    player.state = RUNNING;
                    player.frame = 0; // Reseta o frame ao entrar no estado RUNNING
                    player.maxFrames = 8;
                    player.frameTime = 0.1f;
                }
            }

            if (IsKeyDown(KEY_D)) {
                player.x += MOVE_SPEED;
                movingHorizontal = true;
                movingLeft = false;
                player.flipRight = true;
                player.isJumping = false;
                
                if (player.state != RUNNING) {
                    player.state = RUNNING;
                    player.frame = 0;
                    player.maxFrames = 8;
                    player.frameTime = 0.1f;
                }
            }

            if (IsKeyDown(KEY_R)) {
                if (player.state != ATTACK && !player.isJumping) { // Prevent attacking while jumping
                    player.state = ATTACK;
                    player.frame = 0;
                    player.maxFrames = 5;
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

            aplica_gravidade_player(&player, platforms, total_platform_count, deltaTime);
            
            if(player.y > 180){
                player.lives = 0;
                gameover = true;
            }
            printf("player.y: %d\n", player.y);

            UpdatePlayerAnimation(&player, deltaTime);

            DrawTexture(background_texture, background_x, 0, WHITE );
            DrawTexture(background2_texture, background2_x, 0, WHITE );
            UpdateEnemies(enemies, MAX_ENEMIES, player, platforms, total_platform_count);
            DrawPlayer(player);

            for(int i = 0; i < MAX_ENEMIES; i++) {
                if(enemies[i].isActive) {
                    DrawEnemy(enemies[i].enemy);
                }
            }
            
            // Draw player collision box
            Rectangle collision_box = {
                .x = player.x + (player.width * 2),
                .y = player.y + (player.height * 6) - 10,
                .width = player.width,
                .height = 10
            };
            DrawRectangleRec(collision_box, ColorAlpha(GREEN, 0.5f));

            Rectangle collision_box_enemy = {
                .x = enemy.x + (player.width * 2),
                .y = enemy.y + (player.height * 6) - 10,
                .width = enemy.width * 3,
                .height = 10
            };
            DrawRectangleRec(collision_box_enemy, ColorAlpha(YELLOW, 0.5f));


            Rectangle platform_collision = {
                .x = platform_collision.x,
                .y = platform_collision.y,
                .width = platform_collision.width / 3,
                .height = platform_collision.height
            };
            DrawRectangleRec(platform_collision, ColorAlpha(PINK, 0.5f));

            // Draw player bounds
            Rectangle player_bounds = {
                .x = player.x,
                .y = player.y,
                .width = player.width * 6,
                .height = player.height * 6
            };
            DrawRectangleLines(player_bounds.x, player_bounds.y, 
                            player_bounds.width, player_bounds.height, RED);

            //desenhar floor/platform
            for (int i = 0; i < total_platform_count; i++) {
                Texture2D platform_texture;
                
                // Alterna entre as texturas conforme o tipo da plataforma
                if (platforms[i].type == FLOOR) {
                    platform_texture = platform1_texture;  // Usa `platform1_texture` para FLOOR
                } else {
                    platform_texture = platform2_texture;  // Usa `platform2_texture` para PLATFORM
                }

                // Desenha a textura atual na posição correspondente da plataforma
                DrawTexture(platform_texture, platforms[i].x, platforms[i].y - whitespace, WHITE);
            }


            if (player.invencivel) {
                player.invencibilidadeTimer -= GetFrameTime();
                if (player.invencibilidadeTimer <= 0) {
                    player.invencivel = false;  // Invencibilidade expirada
                }
            }

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].isActive && enemies[i].enemy.isAttacking && !player.invencivel) {
                    if (!player.invencivel) {
                        if (player.lives > 0) {
                            player.lives--;
                            player.invencivel = true;  // Ativa invencibilidade temporária
                            player.invencibilidadeTimer = 1.0f;  // Define um tempo de invencibilidade de 1 segundo
                            printf("Player lives: %d\n", player.lives);
                        }
                        else if(player.lives == 0){
                            isGameOver = true;
                        }
                    }
                }
            }
            DrawLives(player, camera);
            DrawTimer(camera, elapsedTime);
            if (isGameOver){
                EndMode2D();
                //ClearBackground(BLACK);
                const char* text = "Game Over!";
                int textWidth = MeasureText(text, 40);

                DrawText(text, 
                        (GetScreenWidth() - textWidth) / 2,  // Center X
                        GetScreenHeight() / 2 - 20,          // Center Y
                        40, 
                        RED);
            }
        }
        EndDrawing();
        
    }
    // unload texturas
    UnloadTexture(player.idleTexture);
    UnloadTexture(player.runTexture);
    UnloadTexture(player.jumpTexture);
    UnloadTexture(player.attackTexture);
    UnloadTexture(enemy.idleTexture);
    UnloadTexture(enemy.runTexture);
    UnloadTexture(enemy.attackTexture);
    UnloadTexture(backgroundTitle);
    UnloadTexture(floor_piece_texture);
    UnloadFont(fontePersonalizada);
    UnloadTexture(platform1_texture);
    UnloadTexture(platform2_texture);
    UnloadTexture(background_texture);
    UnloadTexture(background2_texture);

    // fecha a janela
    CloseWindow();
    return 0;
}
/* if(gamewin == 1) {
    char nome_player[20];
    char c;
    int i = 0, j = 10;
    loadwinnerlist();
    printf("Escreva seu nome e entre para a Lista de Vencedores: ");

    while ((c = getchar()) != '\n' && i < 19) {
      if(isalnum(c) != 0) {
        nome_player[i] = c;
        i++;
      }
    }

    nome_player[i] = '\0';
    fflush(stdout);
    printf("%s, tempo de jogo: %d ticks\n\n", nome_player, play_time);
    add_jogador(&head, nome_player, play_time);
    
    play_time = elapsedTime?*/