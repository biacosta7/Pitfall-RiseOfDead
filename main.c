#include <SDL2/SDL.h>
#include <stdio.h>

#define FRAME_DELAY 150
#define GRAVIDADE 2
#define PULO_ALTURA 10

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
    SDL_Texture* texture;
    SDL_RendererFlip flip;
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
    SDL_Rect destRect = { zumbi->x, zumbi->y, 64, 64 }; // Dimensão do zumbi
    SDL_RenderCopyEx(renderer, zumbi->texture, NULL, &destRect, 0, NULL, zumbi->flip);
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

// Função para atualizar a posição do personagem e aplicar a gravidade
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

// Função para atualizar a posição do zumbi com base na posição do jogador
void updateZombiePosition(struct Zumbi* zumbi, struct Personagem* player, int groundY) {
    // Define a posição fixa do chão para o zumbi
    zumbi->y = groundY - 64;  // Mantém o zumbi no chão

    // Ajusta a direção do zumbi para seguir o jogador
    if (zumbi->x < player->x) {
        zumbi->velocityX = 3;
        zumbi->flip = SDL_FLIP_NONE;
    } else if (zumbi->x > player->x) {
        zumbi->velocityX = -3;
        zumbi->flip = SDL_FLIP_HORIZONTAL;
    } else {
        zumbi->velocityX = 0;
    }

    // Atualiza a posição horizontal do zumbi
    zumbi->x += zumbi->velocityX;
}

// Função para atualizar o quadro atual da animação
void updateAnimation(struct Personagem* personagem) {
    if (SDL_GetTicks() - personagem->lastFrameTime > FRAME_DELAY) {
        personagem->lastFrameTime = SDL_GetTicks();
        int totalFrames = personagem->currentTexture == personagem->textureRun ? personagem->totalFramesRun :
                          (personagem->currentTexture == personagem->textureJump ? personagem->totalFramesJump : personagem->totalFramesIdle2);
        personagem->currentFrame = (personagem->currentFrame + 1) % totalFrames;
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
        loadTexture("./assets/zombie-h/zombie-solo.bmp", renderer),
        SDL_FLIP_NONE
    };

    while (1) {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);

        SDL_RenderClear(renderer);

        SDL_Rect bgRect = { 0, groundY, windowWidth, windowHeight - groundY };
        SDL_RenderCopy(renderer, backgroundTextureNormal, NULL, &bgRect);

        updatePlayerPosition(&player, groundY);
        updateAnimation(&player);
        drawCharacter(&player, renderer);

        updateZombiePosition(&zumbi, &player, groundY);
        drawZombie(&zumbi, renderer);

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(backgroundTextureNormal);
                SDL_DestroyTexture(player.textureIdle2);
                SDL_DestroyTexture(player.textureRun);
                SDL_DestroyTexture(player.textureJump);
                SDL_DestroyTexture(zumbi.texture);
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
