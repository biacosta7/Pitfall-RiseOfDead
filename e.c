int player_na_plataforma(Player player, Platform platforms[], int total_ground_count, bool *ultimaPlataforma) {
    
    if (platforms == NULL || ultimaPlataforma == NULL) {
        return -1;
    }

    Rectangle player_rec = {
        .x = player.x + (player.width * 2),
        .y = player.y + (player.height * 6) - 10,
        .width = player.width,
        .height = 10  // Small height for ground detection
    };

    for(int i = 0; i < total_ground_count; i++) {
        Rectangle platform_rec = {
            .x = platforms[i].x,
            .y = platforms[i].y,
            .width = platforms[i].width,
            .height = platforms[i].height
        };

        if(CheckCollisionRecs(player_rec, platform_rec)) {
            if (platforms[i].type == FLOOR){
                if(platforms[i].x >= 12400){ //chegando no fim do mundo
                    *ultimaPlataforma = true;
                    printf("platform X: %d\n", platforms[i].x);
                }
                return i;
            }
        }
    }
    return -1;
}

void aplica_gravidade_player(Player *player, Platform platforms[], int total_ground_count, float deltaTime, bool *ultimaPlataforma) {

    const float MAX_FALL_SPEED = 10.0f;
    
    int current_platform = player_na_plataforma(*player, platforms, total_ground_count, ultimaPlataforma);
    bool on_floor = (current_platform != -1); // Player is over a FLOOR type pit
    
    // Apply gravity if not on floor, regardless of jump state
    if (!on_floor || player->isJumping) {
        player->velocityY += GRAVITY * deltaTime;
    }

    // Clamp fall speed
    if (player->velocityY > MAX_FALL_SPEED) {
        player->velocityY = MAX_FALL_SPEED;
    }
    
    // Update position
    player->y += player->velocityY * deltaTime * 60.0f;
    
    // Check floor collision again after movement
    current_platform = player_na_plataforma(*player, platforms, total_ground_count, ultimaPlataforma);

    if (current_platform != -1) {
        // If colliding with floor
        if(ultimaPlataforma){
            if (player->velocityY != 0) {
                player->canJump = false;
            }
        }
        else {
            if (player->velocityY > 0) {  // Only if moving downward
                player->isJumping = false;
                player->y = platforms[current_platform].y - (player->height * 6);
                player->velocityY = 0.0;
                player->canJump = true;    
            }
        }
            
    } else {
        if (player->velocityY != 0) {
            player->canJump = false;
        }
    }
}


int main(void){
    GamePhase currentPhase = PHASE_ONE;
    bool isFinalPhaseTriggered = false;
    bool isGameOver = false;
    bool ultimaPlaforma = false;
    // cria window
    int screenWidth = SCREEN_WIDTH * SCALE_FACTOR;
    int screenHeight = SCREEN_HEIGHT * SCALE_FACTOR;
    InitWindow(screenWidth, screenHeight, "Pitfall - Rise Of Dead");
    //InitAudioDevice();
    double startTime = 0.0;
    bool timeStarted = false;
    Texture2D backgroundTitle = LoadTexture("assets/map/layers/initial-bg.png");
    Texture2D floor_texture = LoadTexture("assets/map/floor.png");
    Texture2D pit2_texture = LoadTexture("assets/obstaculos/a.png");
    Texture2D background_texture = LoadTexture( "assets/map/layers/bg1.png" );
    Texture2D background2_texture = LoadTexture( "assets/map/layers/bg2.png" );
    Texture2D finalfloor_texture = LoadTexture( "assets/map/buraco-fim.png" );
    Texture2D potionTextures[NUM_POTIONS];
    potionTextures[0] = LoadTexture("assets/potions/potion-gold-solo.png");    
    potionTextures[1] = LoadTexture("assets/potions/potion-red-solo.png");  
    potionTextures[2] = LoadTexture("assets/potions/potion-purple-solo.png");  

    Texture2D PotionIcon = LoadTexture("assets/potions/potion-gold-solo.png");

    //Music music = LoadMusicStream("assets/sounds/thriller.wav");
    Texture2D zombiehand_texture = LoadTexture( "assets/obstaculos/zombiehand.png" );
       // Inicializa as poções
    Potion potions[NUM_POTIONS];
    srand(time(NULL));
    for (int i = 0; i < NUM_POTIONS; i++) {
        potions[i].rect = (Rectangle){rand() % (SCREEN_WIDTH - 50), rand() % (SCREEN_HEIGHT - 50), 40, 40};
        potions[i].texture = potionTextures[i];
        potions[i].active = true;
    }
    
    int potionsCollected = 0;

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
    int num_backgrounds_needed = (worldWidth / background_width) + 1;

    // definicões da plataforma
    int whitespace = 30; // espaco em branco da imagem, essa "margem/padding" do topo

    int platform_height = 190; // altura do chão
    int platform_width = 200; // tamanho (largura) de cada plataforma

    int total_ground_count = ceil((float)worldWidth / (float)platform_width); // calcula quantos pedacos de chão são necessários pra cobrir toda a largura do mundo
    
    // criando chão (floor)
    Platform platforms[total_ground_count];
    
    // Alternância das platforms sem espaços entre elas
    int platform_x = 0;  // acumula 

    for(int i=0; i < total_ground_count; i++){
        platforms[i].width = platform_width;
        platforms[i].height = platform_height;
        platforms[i].y = screenHeight - platform_height + whitespace;
        platforms[i].x = platform_x;
        if(i%7 == 6){ 
            platforms[i].type = PIT; // Buraco
        } else {
            platforms[i].type = FLOOR; // Chão
        }
        platform_x += platform_width;
    }

    InitZombieHands(zombie_hands, MAX_ZOMBIE_HANDS, screenWidth, screenHeight, zombiehand_texture, platform_height);
    bool colidiuHand = false;

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
        .hurtTexture = LoadTexture("assets/zombie/Hurt.png"),
        .deadTexture = LoadTexture("assets/zombie/Dead.png"),
        .lives = MAX_LIVES,
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
            
            DrawText(tituloDoJogo, 480, 40, 30, GREEN); //posicao X, posicao Y, tamanho fonte, cor
            DrawText(historiaDoJogo, 300, 150, 20, LIGHTGRAY);
            DrawText("Pressione ENTER para iniciar a corrida!", 390, 540, 25, DARKGREEN);
        }
        else if(gameState == GAMEPLAY){
            //PlayMusicStream(music);
            //UpdateMusicStream(music);
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

            const float MOVE_SPEED = 5.0f; // constante velocidade player

            // movimento player
            if (IsKeyPressed(KEY_W) && !player.isJumping) {
                player.velocityY = -400.0f * GetFrameTime();
                player.isJumping = true;
                player.canJump = false;
                player.state = JUMPING;
                player.frame = 0;
                player.maxFrames = 8;
                player.frameTime = 0.2f;
                player.isAttacking = false;
            }
            
            // Handle horizontal movement
            if (IsKeyDown(KEY_A)) {
                player.x -= MOVE_SPEED;
                movingHorizontal = true;
                movingLeft = true;
                player.flipRight = false;
                player.isJumping = false;
                player.isAttacking = false;

                if (player.state != RUNNING) {
                    player.state = RUNNING;
                    player.frame = 0; // Reseta o frame ao entrar no estado RUNNING
                    player.maxFrames = 8;
                    player.frameTime = 0.1f;
                }
                if(player.y < 176 || player.y > 176){
                    player.isJumping = true;
                }
            }

            if (IsKeyDown(KEY_D)) {
                player.x += MOVE_SPEED;
                movingHorizontal = true;
                movingLeft = false;
                player.flipRight = true;
                player.isJumping = false;
                player.isAttacking = false;
                
                if (player.state != RUNNING) {
                    player.state = RUNNING;
                    player.frame = 0;
                    player.maxFrames = 8;
                    player.frameTime = 0.1f;
                }
                if(player.y < 176 || player.y > 176){
                    player.isJumping = true;
                }
            }
            if (IsKeyDown(KEY_R)) {
                if (player.state != ATTACK && !player.isJumping) { // Prevent attacking while jumping
                    player.state = ATTACK;
                    player.frame = 0;
                    player.maxFrames = 5;
                    player.frameTime = 0.1f;
                    player.isAttacking = true;
                }
            }
            else if (!movingHorizontal && !player.isJumping) {
                player.isAttacking = false;
                if (player.state != IDLE) {
                    player.state = IDLE;
                    player.frame = 0;
                    player.maxFrames = 5;
                    player.frameTime = 0.3f;
                }
            }

            aplica_gravidade_player(&player, platforms, total_ground_count, deltaTime, &ultimaPlaforma);
            
            if(player.y > 180){
                player.lives = 0;
                isGameOver = true;
            }

            // limites do player
            if (player.x < 0) {
                player.x = 0;  // Stop at left edge
            } else if (player.x > (worldWidth - (player.width * 3))) {  // Accounting for player bounds
                player.x = (worldWidth - (player.width * 3));  // Stop at right edge
            }

            UpdatePlayerAnimation(&player, deltaTime);

            // primeiro background layer (moves slower)
            for (int i = 0; i < num_backgrounds_needed; i++) {
                DrawTexture(background_texture, i * background_width - (camera.target.x * 0.3f), 0, WHITE);
            } // mais devadar (0.3 = 30% da camera speed)

            // segundo background layer (moves faster)
            float parallax_factor = 0.3f;  
            for (int i = 0; i < num_backgrounds_needed; i++) {
                DrawTexture(background2_texture, i * background_width - camera.target.x, 0, WHITE);
            }
            
            UpdateEnemies(enemies, MAX_ENEMIES, player, platforms, total_ground_count);
            UpdateZombieHands(zombie_hands, MAX_ZOMBIE_HANDS, player, &colidiuHand);
            DrawPlayer(player);
            // Verifica colisão com as poções

            for(int i = 0; i < MAX_ENEMIES; i++) {
                if(enemies[i].isActive) {
                    DrawEnemy(enemies[i].enemy);
                }
            }
            
            // Draw player collision box
            // Rectangle collision_box = {
            //     .x = player.x + (player.width * 2),
            //     .y = player.y + (player.height * 6) - 10,
            //     .width = player.width,
            //     .height = 10
            // };
            // DrawRectangleRec(collision_box, ColorAlpha(GREEN, 0.5f));

            // In your draw code
            //DrawRectangleLines(hand_rec.x, hand_rec.y, hand_rec.width, hand_rec.height, RED);


            // Draw player bounds
            Rectangle player_bounds = {
                .x = player.x,
                .y = player.y,
                .width = player.width * 3,
                .height = player.height * 6
            };
            DrawRectangleLines(player_bounds.x, player_bounds.y, 
                            player_bounds.width, player_bounds.height, RED);

            for (int i = 0; i < NUM_POTIONS; i++) {
                if (potions[i].active && CheckCollisionRecs(player_bounds, potions[i].rect)) {
                    potions[i].active = false;
                    potionsCollected++; // Incrementa o contador


                    // Reposiciona a poção em um local aleatório
                    potions[i].rect.x = rand() % (SCREEN_WIDTH - (int)potions[i].rect.width);
                    potions[i].rect.y = rand() % (SCREEN_HEIGHT - (int)potions[i].rect.height);
                    potions[i].active = true;
                }
            }

            for (int i = 0; i < NUM_POTIONS; i++) {
            if (potions[i].active) {
                DrawTexture(potions[i].texture, potions[i].rect.x, potions[i].rect.y, WHITE);
                }
            }
                // Desenha o contador no canto superior direito
            DrawTexture(PotionIcon, SCREEN_WIDTH - 100, 20, WHITE);
            DrawText(TextFormat("%d", potionsCollected), SCREEN_WIDTH - 50, 30, 20, DARKGRAY);

            //desenhar floor/pit
            for (int i = 0; i < total_ground_count; i++) {
                Texture2D platform_texture;
                
                // Alterna entre as texturas conforme o tipo da platform
                if (platforms[i].type == FLOOR) {
                    // draw hand
                    DrawZombieHands(zombie_hands, MAX_ZOMBIE_HANDS);
                    platform_texture = floor_texture;  // Usa `floor_texture` para FLOOR
                } else {
                    platform_texture = pit2_texture;  // Usa `pit2_texture` para PIT
                }

                // Desenha a textura atual na posição correspondente da platform
                DrawTexture(platform_texture, platforms[i].x, platforms[i].y - whitespace, WHITE);
            }

            

            if (player.invencivel) {
                player.invencibilidadeTimer -= GetFrameTime();
                if (player.invencibilidadeTimer <= 0) {
                    player.invencivel = false;  // Invencibilidade expirada
                }
            }
            

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].isActive) {
                    if(player.isAttacking && !enemies[i].enemy.invencivel){
                        if(enemies[i].enemy.lives > 0){
                            enemies[i].enemy.state = HURT;
                            enemies[i].enemy.lives--;
                            enemies[i].enemy.invencivel = true;  // Ativa invencibilidade temporária
                            enemies[i].enemy.invencibilidadeTimer = 0.5f;  // Define um tempo de invencibilidade de 1 segundo
                            //printf("ENEMY LIVES: %d\n", enemies[i].enemy.lives);
                        }
                        else if(enemies[i].enemy.lives == 0){
                            enemies[i].enemy.isAttacking = false;
                            enemies[i].enemy.isDead = true;
                            //printf("MORREU ZUMBIZINHO\n");
                        }
                    }
                    if(enemies[i].enemy.isAttacking && !player.invencivel && !player.isAttacking){
                        if (player.lives > 0) {
                            player.lives--;
                            player.invencivel = true;  // Ativa invencibilidade temporária
                            player.invencibilidadeTimer = 1.0f;  // Define um tempo de invencibilidade de 1 segundo
                        }
                        else if(player.lives == 0){
                            isGameOver = true;
                            //PauseMusicStream(music);
                        }
                    }

                    if (enemies[i].enemy.invencivel) {
                        enemies[i].enemy.invencibilidadeTimer -= GetFrameTime();
                        if (enemies[i].enemy.invencibilidadeTimer <= 0) {
                            enemies[i].enemy.invencivel = false;  // Invencibilidade expirada
                        }
                    }

                }
                if(colidiuHand && !player.invencivel){
                    printf("A MAO PEGOU O PE\n");
                    if (player.lives > 0) {
                        player.lives--;
                        player.invencivel = true;  // Ativa invencibilidade temporária
                        player.invencibilidadeTimer = 1.0f;  // Define um tempo de invencibilidade de 1 segundo
                    }
                    else if(player.lives == 0){
                        isGameOver = true;
                        //PauseMusicStream(music);
                    }
                } 
                // if(enemy.state == DEAD && enemy.frame == 4){
                //     enemy.x = -1000;
                // }
            }
            DrawLives(player, camera);
            DrawTimer(camera, elapsedTime);


            // Based on potions collected and player position
            if (potionsCollected == 3 && player.x >= worldWidth * 0.9f && !isFinalPhaseTriggered) {
                currentPhase = FINAL_PHASE;
                isFinalPhaseTriggered = true;
                // Initialize final phase elements
            }

            //printf("player x: %d\n", player.x);
            DrawTexture(finalfloor_texture, 12550, screenHeight - 230, WHITE);

            if (isGameOver){
                EndMode2D();
                ClearBackground(BLACK);
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
    UnloadTexture(floor_texture);
    UnloadTexture(pit2_texture);
    UnloadTexture(background_texture);
    UnloadTexture(background2_texture);
    UnloadTexture(PotionIcon);
    UnloadTexture(finalfloor_texture);
    CloseWindow();
    
    return 0;
}
