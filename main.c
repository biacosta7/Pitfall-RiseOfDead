#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#define FRAME_DELAY 150
#define GRAVIDADE 2
#define PULO_ALTURA 20
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 400;

// gcc -I/usr/local/include/SDL2 ./main.c -o test.exe -L/usr/local/lib -lSDL2
// gcc -o teste main.c -lSDL2
// gcc main.c -o test_sdl2 -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -mmacosx-version-min=15.0

struct Personagem {
    int x, y;
    int velocityX, velocityY;
    int onGround;
    SDL_Texture* textureIdle2;
    SDL_Texture* textureRun;
    SDL_Texture* textureJump;
    SDL_Texture* textureattack;
    SDL_Texture* currentTexture;
    SDL_RendererFlip flip;
    int frameWidth, frameHeight;
    int currentFrame;
    int totalFramesIdle2;
    int totalFramesRun;
    int totalFramesattack;
    int totalFramesJump;
    Uint32 lastFrameTime;
};

struct Zumbi {
    int x, y;
    float velocityX, velocityY;
    int onGround;
    SDL_Texture* textureIdle;
    SDL_Texture* textureRun;
    SDL_Texture* currentTexture;
    SDL_RendererFlip flip;
    int frameWidth, frameHeight;
    int currentFrame;
    int totalFramesIdle;
    int totalFramesRun;
    Uint32 lastFrameTime;
};

// Função para carregar uma textura a partir de um arquivo BMP
SDL_Texture* loadTexture(const char* filePath, SDL_Renderer* renderer) {
    SDL_Surface* surface = SDL_LoadBMP(filePath);
    if (!surface) {
        printf("Erro ao carregar BMP: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_FreeSurface(surface);
    return texture;
}

void renderBackground(SDL_Renderer* renderer, SDL_Texture* bgTexture, int bgX) {
    // Renderiza tela preta se o fundo estiver fora da tela
    if (bgX <= 0) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Define a cor preta
        SDL_Rect blackRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
        SDL_RenderFillRect(renderer, &blackRect); // Desenha o retângulo preto
    }

    // Renderiza o fundo
    SDL_Rect srcRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Rect destRect = {bgX, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    SDL_RenderCopy(renderer, bgTexture, &srcRect, &destRect);

    // Renderiza o fundo repetido
    destRect.x = bgX + SCREEN_WIDTH; // posição para a próxima parte do fundo
    SDL_RenderCopy(renderer, bgTexture, &srcRect, &destRect);
}

// Função para desenhar o personagem na tela
void drawCharacter(struct Personagem* personagem, SDL_Renderer* renderer) {
    SDL_Rect srcRect = { personagem->currentFrame * personagem->frameWidth, 0, personagem->frameWidth, personagem->frameHeight };
    SDL_Rect destRect = { (SCREEN_WIDTH / 2) - (personagem->frameWidth / 2), personagem->y, personagem->frameWidth, personagem->frameHeight }; // Centraliza o personagem
    SDL_RenderCopyEx(renderer, personagem->currentTexture, &srcRect, &destRect, 0, NULL, personagem->flip);
}

// Função para desenhar o zumbi na tela
void drawZombie(struct Zumbi* zumbi, SDL_Renderer* renderer) {
    SDL_Rect srcRect = { zumbi->currentFrame * zumbi->frameWidth, 0, zumbi->frameWidth, zumbi->frameHeight };
    SDL_Rect destRect = { zumbi->x, zumbi->y, zumbi->frameWidth, zumbi->frameHeight };
    SDL_RenderCopyEx(renderer, zumbi->currentTexture, &srcRect, &destRect, 0, NULL, zumbi->flip);
}

// Função para lidar com entrada de movimento e pulo
void handleInput(struct Personagem* player, int input, int bgX) {
    if (input == SDLK_w && player->onGround) {
        player->velocityY = -PULO_ALTURA;
        player->currentTexture = player->textureJump;
        player->currentFrame = 0;
        player->onGround = 0;
    }
      if (input==SDLK_r) {
        
        player->currentTexture=player->textureattack;
        player->totalFramesattack= 5;
        }

    if (input == SDLK_d) {
        player->velocityX = 10;
        player->flip = SDL_FLIP_NONE;
        if (player->onGround) {
            player->currentTexture = player->textureRun;
        }
    }
    if (input == SDLK_a) {
        player->velocityX = -10;
        player->flip = SDL_FLIP_HORIZONTAL;
        if (player->onGround) {
            player->currentTexture = player->textureRun;
        }
    }
}

void handleKeyUp(struct Personagem* player, int input) {
    if (input == SDLK_a || input == SDLK_d) {
        player->velocityX = 0;
        if (player->onGround) {
            player->currentTexture = player->textureIdle2;
            player->totalFramesIdle2 = 5;
        }
    }
}

// atualiza a posição do personagem e aplicar a gravidade
void updatePlayerPosition(struct Personagem* personagem, int groundY, int bgX) {
    personagem->x += personagem->velocityX;

    // Impede que o personagem ultrapasse o limite do fundo
    if (personagem->x < -bgX) {
        personagem->x = -bgX; // Limita a posição do jogador ao limite do fundo
    }

    if (!personagem->onGround) { 
        personagem->velocityY += GRAVIDADE;
        personagem->y += personagem->velocityY;
        if (personagem->y >= groundY - personagem->frameHeight) {
            personagem->y = groundY - personagem->frameHeight;
            personagem->onGround = 1;
            personagem->velocityY = 0;
            if (personagem->velocityX == 0) {
                personagem->currentTexture = personagem->textureIdle2;
            } else {
                personagem->currentTexture = personagem->textureRun;
            }
        }
    }
}

void updateBackgroundPosition(int* backgroundX, struct Personagem* player, float parallaxSpeed) {
    *backgroundX -= player->velocityX * parallaxSpeed;

    if (*backgroundX > 0) {
        *backgroundX = 0;
    }

    // Reseta a posição do fundo para criar o efeito de repetição
    if (*backgroundX <= -SCREEN_WIDTH) {
        *backgroundX += SCREEN_WIDTH;
    }
}

// função para atualizar a posição do zumbi com base na posição do player
void updateZombiePosition(struct Zumbi* zumbi, struct Personagem* player, int groundY) {
    zumbi->y = groundY - zumbi->frameHeight;

    int playerCenterX = SCREEN_WIDTH / 2; // posição central do player

    // Define a distância mínima base
    int distance = 85;

    // Aumenta a distância se o jogador estiver se movendo para a direita
    if (player->velocityX > 0) {
        distance += player->velocityX * 5;
    } else if (player->velocityX < 0) { // Reduz a distância se o jogador estiver se movendo para a esquerda
        distance = 85;
    }
    // Se o zumbi está à esquerda do player menos a distância, ele se move para a direita
    if (zumbi->x < playerCenterX - distance) {
        zumbi->velocityX = 1; // Move para a direita
        zumbi->flip = SDL_FLIP_NONE;
        zumbi->currentTexture = zumbi->textureRun; // textura de corrida
        zumbi->totalFramesRun= 7;
    }
    // Se o zumbi está à direita do player mais a distância, ele se move para a esquerda
    else if (zumbi->x > playerCenterX + distance) {
        zumbi->velocityX = -0.4f; // Move para a esquerda
        zumbi->flip = SDL_FLIP_HORIZONTAL;
        zumbi->currentTexture = zumbi->textureRun; 
    } 
    // Se o zumbi está na faixa desejada em relação ao player, ele para
    else {
        if (player->velocityX != 0 && distance != 85) { // player está se movendo pra direita
            zumbi->velocityX = -0.4f;
            zumbi->currentTexture = zumbi->textureRun;
        }
        else { 
            zumbi->velocityX = 0; // Para
            zumbi->currentTexture = zumbi->textureIdle; 
        }
    }

    // Atualiza a posição do zumbi com base na sua velocidade
    zumbi->x += zumbi->velocityX;

    // Impede que o zumbi ultrapasse a tela
    if (zumbi->x < 0) {
        zumbi->x = 0;
    } else if (zumbi->x > SCREEN_WIDTH - zumbi->frameWidth) {
        zumbi->x = SCREEN_WIDTH - zumbi->frameWidth;
    }

}

// função para atualizar o quadro atual da animação
void updateAnimation(struct Personagem* personagem) {
    // verifica se o tempo para trocar de frame foi atingido
    if (SDL_GetTicks() - personagem->lastFrameTime > FRAME_DELAY) {
        personagem->lastFrameTime = SDL_GetTicks(); // atualiza o tempo do último frame
        
        int totalFrames;
        
        // verifica qual é a textura atual e define o número de frames correspondente
        if (personagem->currentTexture == personagem->textureRun) {
            totalFrames = personagem->totalFramesRun;
        } else if (personagem->currentTexture == personagem->textureJump) {
            totalFrames = personagem->totalFramesJump;
        } else if (personagem->currentTexture == personagem->textureattack) {
            totalFrames = personagem->totalFramesattack;
        } else {
            totalFrames = personagem->totalFramesIdle2;
        }
        
        personagem->currentFrame = (personagem->currentFrame + 1) % totalFrames; // atualiza o frame atual do personagem
    }
}


void updateAnimationZombie(struct Zumbi* zumbi){
    // verifica se o tempo para trocar de frame foi atingido
    if (SDL_GetTicks() - zumbi->lastFrameTime > FRAME_DELAY) {
        zumbi->lastFrameTime = SDL_GetTicks(); // atualiza o tempo do último frame
        
        int totalFrames;
        
        // verifica se a textura atual é de corrida ou parado
        if (zumbi->currentTexture == zumbi->textureRun) {
            totalFrames = zumbi->totalFramesRun;
        } else {
            totalFrames = zumbi->totalFramesIdle;
        }
        
        zumbi->currentFrame = (zumbi->currentFrame + 1) % totalFrames; // atualiza o frame atual do zumbi
    }
}

int main(int argc, char* argv[]) {
    int input;
    int isFullscreen = 0;
    int bgX = 0; // posição inicial do fundo

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    int windowWidth = SCREEN_WIDTH;
    int windowHeight = SCREEN_HEIGHT;

    SDL_Window* window = SDL_CreateWindow("Pitfall: Rise Of Dead", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* bgTexture = loadTexture( "./assets/map/layers/forest-1.bmp", renderer);

    int groundY = (int)(windowHeight * 0.8);

    struct Personagem player = {
        0, groundY - 128, 0, 0, 1,
        loadTexture("./assets/player/idle2.bmp", renderer),
        loadTexture("./assets/player/run.bmp", renderer),
        loadTexture("./assets/player/jump.bmp", renderer),
        loadTexture("./assets/player/Attack_2.bmp", renderer),
        NULL, SDL_FLIP_NONE, 128, 128, 0, 6, 8, 4, 0
    };
    player.currentTexture = player.textureIdle2;
    player.x = (SCREEN_WIDTH / 2) - (player.frameWidth / 2); // Mantém o personagem centralizado

    player.y = groundY - player.frameHeight;

    struct Zumbi zumbi = {
        10, groundY - 128, 0, 0, 1,
        loadTexture("./assets/zombie/Idle.bmp", renderer),
        loadTexture("./assets/zombie/Run.bmp", renderer),
        NULL, SDL_FLIP_NONE, 96, 96, 0, 4, 8
    };
    zumbi.currentTexture = zumbi.textureIdle;
    while (1) {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);

        SDL_RenderClear(renderer);

        int playerPositionX = 0; 

        playerPositionX += player.velocityX; // atualize a posição do jogador com base na velocidade

        // Atualizações do jogo
        updatePlayerPosition(&player, groundY, bgX);
        updateZombiePosition(&zumbi, &player, groundY);
        updateBackgroundPosition(&bgX, &player, 0.6f);
        updateAnimation(&player);
        updateAnimationZombie(&zumbi);

        // Renderização
        SDL_RenderClear(renderer);
        renderBackground(renderer, bgTexture, bgX);
        drawCharacter(&player, renderer);
        drawZombie(&zumbi, renderer);

        SDL_RenderPresent(renderer);

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(bgTexture);
                SDL_DestroyTexture(player.textureIdle2);        
                SDL_DestroyTexture(player.textureRun);
                SDL_DestroyTexture(player.textureJump);
                SDL_DestroyTexture(player.textureattack);
                SDL_DestroyTexture(zumbi.textureIdle);
                SDL_DestroyTexture(zumbi.textureRun);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
            if (event.type == SDL_KEYDOWN) {
                input = event.key.keysym.sym;
                handleInput(&player, input, bgX);
            }
            if (event.type == SDL_KEYUP) {
                input = event.key.keysym.sym;
                handleKeyUp(&player, input);
            }
        }
        SDL_Delay(16);

        
    }
}
