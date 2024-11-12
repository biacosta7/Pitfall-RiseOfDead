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
    //int speed;
    //int jump_strength;
    float velocityY; // velocidade vertical (para controlar o pulo)
    bool isJumping;
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
    bool walking;
    int lives;
    Texture2D heartTexture3;
    Texture2D heartTexture2;
    Texture2D heartTexture1;
    bool invencivel; //tempo de invencibilidade logo apos levar uma colisao
    float invencibilidadeTimer; // tempo que dura a invencibilidade
    int direction;
} Player;


// struct enemy
typedef struct{
    int x;
    int y;
    int width;
    int height;
    float velocityY;
    PersonagemState state;
    Texture2D idleTexture;
    Texture2D runTexture;
    Texture2D attackTexture;
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

void DrawLives(Player player) {
    Vector2 livesPosition = { 20, 20 }; // Posição fixa no canto superior esquerdo da tela
    if (player.lives == 3) {
        DrawTexture(player.heartTexture3, livesPosition.x, livesPosition.y, WHITE);
    } else if (player.lives == 2) {
        DrawTexture(player.heartTexture2, livesPosition.x, livesPosition.y, WHITE);
    } else if (player.lives == 1) {
        DrawTexture(player.heartTexture1, livesPosition.x, livesPosition.y, WHITE);
    }
}

// atualiza o parallax
void UpdateDrawParallax(BackgroundLayer *layers, int layerCount, float deltaTime, int screenWidth, int screenHeight, bool movingHorizontal, bool movingLeft, Camera2D camera) {
    for (int i = 0; i < layerCount; i++) {
        // Escala com base na altura da tela
        float scale = (float)screenHeight / layers[i].texture.height;

        // Ajusta a posição das camadas conforme a velocidade de movimento
        if (movingHorizontal) {
            if (movingLeft) {
                layers[i].position.x += layers[i].speed * deltaTime;
            } else {
                layers[i].position.x -= layers[i].speed * deltaTime;
            }
        }

        // Reseta a posição se a textura ultrapassar a tela
        if (layers[i].position.x <= -(layers[i].texture.width * scale)) {
            layers[i].position.x = 0;
        }

        // Centraliza horizontalmente caso a camada seja menor que a largura da tela
        float deslocamento_x = 0;
        if (layers[i].texture.width * scale < screenWidth) {
            deslocamento_x = (screenWidth - (layers[i].texture.width * scale)) / 2;
        }

        // Aplica o offset da câmera com intensidade variável para efeito de profundidade
        float parallaxdeslocamento_x = camera.offset.x * (0.1f + 0.05f * i);

        // Desenha a textura duas vezes para criar um scroll contínuo
        DrawTextureEx(layers[i].texture, (Vector2){layers[i].position.x + deslocamento_x + parallaxdeslocamento_x, 0}, 0, scale, WHITE);
        DrawTextureEx(layers[i].texture, (Vector2){layers[i].position.x + (layers[i].texture.width * scale) + deslocamento_x + parallaxdeslocamento_x, 0}, 0, scale, WHITE);
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

int player_na_plataforma(Player player, Platform platforms[], int total_platform_count) {
    Rectangle player_rec = {
        .x = player.x + (player.width * 2),
        .y = player.y + (player.height * 6) - 10,
        .width = player.width * 2,
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
        .y = enemy.y + (enemy.height * 6) - 10,
        .width = enemy.width * 3,
        .height = 10 
    };

    Rectangle player_rec = {
        .x = player.x + (player.width * 2),
        .y = player.y + (player.height * 6) - 10,
        .width = player.width * 3,
        .height = 10 
    };

    if(CheckCollisionRecs(enemy_rec, player_rec)) {
        return true;
    }
    return false;
}

void aplica_gravidade(Player *player, Enemy *enemy, Platform platforms[], int total_platform_count, float deltaTime) {
    const float MAX_FALL_SPEED = 10.0f;  // Maximum falling speed
    
    // Apply gravity with deltaTime
    player->velocityY += GRAVITY * deltaTime;
    enemy->velocityY += GRAVITY * deltaTime;

    // Clamp fall speed
    if (player->velocityY > MAX_FALL_SPEED) {
        player->velocityY = MAX_FALL_SPEED;
    }
    if (enemy->velocityY > MAX_FALL_SPEED) {
        enemy->velocityY = MAX_FALL_SPEED;
    }
    
    // Update position
    player->y += player->velocityY * deltaTime * 60.0f; // Normalize for 60 FPS
    enemy->y += enemy->velocityY * deltaTime * 60.0f;
    
    // Check platform collision
    int current_platform = player_na_plataforma(*player, platforms, total_platform_count);
    int current_platform_enemy = enemy_na_plataforma(*enemy, platforms, total_platform_count);

    if (current_platform != -1) {
        // If colliding with platform
        if (player->velocityY > 0) {  // Only if moving downward
            // Snap to platform top
            player->y = platforms[current_platform].y - (player->height * 6);
            player->velocityY = 0;
            player->isJumping = false;
        }
    } else {
        // If in air, ensure jumping state is set
        if (player->velocityY != 0) {
            player->isJumping = true;
        }
    }
    
    if (current_platform_enemy != -1) {
        // If colliding with platform
        if (enemy->velocityY > 0) {  // Only if moving downward
            // Snap to platform top
            enemy->y = platforms[current_platform].y - (enemy->height * 6);
            enemy->velocityY = 0;
        }
    }

    // enemy->velocityY += GRAVITY * deltaTime;
    //enemy->y += enemy->velocityY * deltaTime;

    // float groundLevel = 190 - player->height * 0.15;
    // if (enemy->y > groundLevel) {
    //     enemy->y = groundLevel;
    //     enemy->velocityY = 0;
    // }
}


int main(void){
    // cria window
    int screenWidth = SCREEN_WIDTH * SCALE_FACTOR;
    int screenHeight = SCREEN_HEIGHT * SCALE_FACTOR;
    InitWindow(screenWidth, screenHeight, "Pitfall - Rise Of Dead");

    BackgroundLayer layers[LAYER_COUNT] = {
        { LoadTexture("assets/map/layers/bg1.png"), (Vector2){0, 0}, 50 },
        { LoadTexture("assets/map/layers/bg3.png"), (Vector2){0, 0}, 100 },
    };
    Texture2D backgroundTitle = LoadTexture("assets/map/layers/initialbackground.png");
    Texture2D floor_piece_texture = LoadTexture( "assets/map/floor.png");
    Font fontePersonalizada = LoadFont("assets/fonts/bloodcrow.ttf");
    Texture2D platform1_texture = LoadTexture("assets/map/platform-floor.png");
    Texture2D platform2_texture = LoadTexture("assets/obstaculos/platform2.png");

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
    
    // definicões da plataforma
    int whitespace = 30; // espaco em branco da imagem, essa "margem/padding"

    int platform_height = 190; // altura do chão
    int platform_width = 180; // tamanho (largura) de cada plataforma

    int total_platform_count = ceil((float)worldWidth / (float)platform_width); // calcula quantos pedacos de chão são necessários pra cobrir toda a largura do mundo
    
    // criando chão (floor)
    Platform platforms[total_platform_count];

    int floor_x = 0;
    int floor_offset = 100;  // Adjust this value to move floor higher or lower
    
    // Alternância das plataformas sem espaços entre elas
    int platform_x = floor_x;  // Continua após o chão // o que é isso hein???

    for(int i=0; i < total_platform_count; i++){
        if(i%6 == 0){
            platforms[i].x = platform_x;
            platforms[i].y = screenHeight - platform_height + whitespace;
            platforms[i].width = platform_width;
            platforms[i].height = platform_height;
            platforms[i].type = PLATFORM; // Plataforma

            platform_x += platform_width;
        } else {
            platforms[i].x = platform_x;
            platforms[i].y = screenHeight - platform_height + whitespace;
            platforms[i].width = platform_width;
            platforms[i].height = platform_height;
            platforms[i].type = FLOOR; // Floor

            platform_x += platform_width;
        }
    }

    // cria player
    Player player = {
        .x = SCREEN_WIDTH / 2,
        .y = 0,
        .width = 64,
        .height = 64,
        .state = IDLE,
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
        .walking = false,
        .direction = 1,
    };
    
    // cria enemy
    Enemy enemy = {
        .x = SCREEN_WIDTH / 2,
        .y = 0,
        .width = 64,
        .height = 64,
        .state = IDLE,
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
            if(IsKeyPressed(KEY_ENTER)){
                gameState = GAMEPLAY;
            }
        }
        else if(gameState == GAMEPLAY){
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

            const float MOVE_SPEED = 5.0f; // constante velocidade player

            // movimento player
            if (IsKeyPressed(KEY_W)) {
                player.velocityY = -10.0f;
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
                player.walking = true;
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
                player.walking = true;
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
                    player.walking = false;
                    player.frame = 0;
                    player.maxFrames = 5;
                    player.frameTime = 0.05f;
                }
            }
            else if (!movingHorizontal && !player.isJumping) {
                player.walking = false;
                if (player.state != IDLE) {
                    player.state = IDLE;
                    player.frame = 0;
                    player.maxFrames = 5;
                    player.frameTime = 0.3f;
                }
            }

            // inimigo seguindo o player
            if (player.x > enemy.x && !enemy_colide_player(enemy, player)){
                enemy.x += 2.0f;
                enemy.flipRight = true;
                if (enemy.state != RUNNING) {
                    enemy.state = RUNNING;
                    enemy.frame = 0;
                    enemy.maxFrames = 7;
                    enemy.frameTime = 0.1f;
                }
            } else if (player.x < enemy.x && !enemy_colide_player(enemy, player)){
                enemy.x -= 2.0f;
                enemy.flipRight = false;
                if (enemy.state != RUNNING) {
                    enemy.state = RUNNING;
                    enemy.frame = 0;
                    enemy.maxFrames = 7;
                    enemy.frameTime = 0.1f;
                }
            } 
            else if(enemy_colide_player(enemy, player)){
                if (enemy.state != ATTACK) {
                    enemy.state = ATTACK;
                    enemy.frame = 0;
                    enemy.maxFrames = 4;
                    enemy.frameTime = 0.1f;
                }
            } //else if{
            //     if (enemy.state != IDLE) {
            //         enemy.state = IDLE;
            //         enemy.frame = 0;
            //         enemy.maxFrames = 8;
            //         enemy.frameTime = 0.3f;
            //     }
            // } 

            aplica_gravidade(&player, &enemy, platforms, total_platform_count, deltaTime);
            
            UpdatePlayerAnimation(&player, deltaTime);
            UpdateEnemyAnimation(&enemy, deltaTime);

            UpdateDrawParallax(layers, LAYER_COUNT, GetFrameTime(), screenWidth, screenHeight, movingHorizontal, movingLeft, camera);
            DrawEnemy(enemy);
            DrawPlayer(player);

            // Draw player collision box
            Rectangle collision_box = {
                .x = player.x + (player.width * 2),
                .y = player.y + (player.height * 6) - 10,
                .width = player.width * 3,
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

            if (colidiu && !player.invencivel) {
                if (player.lives > 0) {
                    player.lives--;
                    player.invencivel = true;  // Ativa invencibilidade temporária
                    player.invencibilidadeTimer = 1.0f;  // Define um tempo de invencibilidade de 1 segundo
                }
                else if(player.lives == 0){
                    DrawText("Game Over!", 600, 400, 40, RED);
                }
            }
            DrawLives(player);
        }
        EndMode2D();
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

    for (int i = 0; i < LAYER_COUNT; i++) {
        UnloadTexture(layers[i].texture);
    }
    // fecha a janela
    CloseWindow();
    return 0;
}