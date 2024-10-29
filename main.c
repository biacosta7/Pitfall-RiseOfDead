#include <SDL2/SDL.h>
#include <stdio.h>

#define FRAME_DELAY 150
#define GRAVIDADE 2
#define PULO_ALTURA 20

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
    SDL_Texture* currentTexture;
    SDL_RendererFlip flip;
    int frameWidth, frameHeight;
    int currentFrame;
    int totalFramesIdle2;
    int totalFramesRun;
    int totalFramesJump;
    Uint32 lastFrameTime;
};

struct Zumbi {
    int x, y;
    int velocityX, velocityY;
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

// Função para desenhar o personagem na tela
void drawCharacter(struct Personagem* personagem, SDL_Renderer* renderer) {
    SDL_Rect srcRect = { personagem->currentFrame * personagem->frameWidth, 0, personagem->frameWidth, personagem->frameHeight };
    SDL_Rect destRect = { personagem->x, personagem->y, personagem->frameWidth, personagem->frameHeight };
    SDL_RenderCopyEx(renderer, personagem->currentTexture, &srcRect, &destRect, 0, NULL, personagem->flip);
}

// Função para desenhar o zumbi na tela
void drawZombie(struct Zumbi* zumbi, SDL_Renderer* renderer) {
    SDL_Rect srcRect = { zumbi->currentFrame * zumbi->frameWidth, 0, zumbi->frameWidth, zumbi->frameHeight };
    SDL_Rect destRect = { zumbi->x, zumbi->y, zumbi->frameWidth, zumbi->frameHeight };
    SDL_RenderCopyEx(renderer, zumbi->currentTexture, &srcRect, &destRect, 0, NULL, zumbi->flip);
}

// Função para lidar com entrada de movimento e pulo
void handleInput(struct Personagem* player, int input) {
    if (input == SDLK_w && player->onGround) {
        player->velocityY = -PULO_ALTURA;
        player->currentTexture = player->textureJump;
        player->currentFrame = 0;
        player->onGround = 0;
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
void updatePlayerPosition(struct Personagem* personagem, int groundY) {
    personagem->x += personagem->velocityX;
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
// função para atualizar a posição do zumbi com base na posição do jogador
void updateZombiePosition(struct Zumbi* zumbi, struct Personagem* player, int groundY) {
    zumbi->y = groundY - zumbi->frameHeight;

    if (zumbi->x < player->x) {
        zumbi->velocityX = 1;
        zumbi->flip = SDL_FLIP_NONE;
        zumbi->currentTexture = zumbi->textureRun;  // altere para a textura de corrida
    } else if (zumbi->x > player->x) {
        zumbi->velocityX = -1;
        zumbi->flip = SDL_FLIP_HORIZONTAL;
        zumbi->currentTexture = zumbi->textureRun;  // altere para a textura de corrida
    } else {
        zumbi->velocityX = 0;
        zumbi->currentTexture = zumbi->textureIdle;  // altere para a textura parada
    }

    zumbi->x += zumbi->velocityX; // atualize a posição do zumbi
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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    const int fixedWindowWidth = 800;
    const int fixedWindowHeight = 400;
    int windowWidth = fixedWindowWidth;
    int windowHeight = fixedWindowHeight;

    SDL_Window* window = SDL_CreateWindow("Pitfall: Rise Of Dead", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* backgroundTextureNormal = loadTexture("./assets/map/forest-ground.bmp", renderer);

    int groundY = (int)(windowHeight * 0.8);

    struct Personagem player = {
        100, groundY - 128, 0, 0, 1,
        loadTexture("./assets/player/idle2.bmp", renderer),
        loadTexture("./assets/player/run.bmp", renderer),
        loadTexture("./assets/player/jump.bmp", renderer),
        NULL, SDL_FLIP_NONE, 128, 128, 0, 6, 8, 4, 0
    };
    player.currentTexture = player.textureIdle2;

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

        SDL_Rect bgRect = { 0, groundY, windowWidth, windowHeight - groundY };
        SDL_RenderCopy(renderer, backgroundTextureNormal, NULL, &bgRect);

        updatePlayerPosition(&player, groundY);
        updateAnimation(&player);
        drawCharacter(&player, renderer);

        updateZombiePosition(&zumbi, &player, groundY);
        updateAnimationZombie(&zumbi);
        drawZombie(&zumbi, renderer);

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(backgroundTextureNormal);
                SDL_DestroyTexture(player.textureIdle2);
                SDL_DestroyTexture(player.textureRun);
                SDL_DestroyTexture(player.textureJump);
                SDL_DestroyTexture(zumbi.textureIdle);
                SDL_DestroyTexture(zumbi.textureRun);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
            if (event.type == SDL_KEYDOWN) {
                input = event.key.keysym.sym;
                handleInput(&player, input);
            }
            if (event.type == SDL_KEYUP) {
                input = event.key.keysym.sym;
                handleKeyUp(&player, input);
            }
        }
        SDL_Delay(16);
    }
}
