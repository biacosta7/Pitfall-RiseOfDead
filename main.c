#include <SDL2/SDL.h>
#include <stdio.h>

#define FRAME_DELAY 150
#define GRAVIDADE 2
#define PULO_ALTURA 10

// gcc -I/usr/local/include/SDL2 ./main.c -o test.exe -L/usr/local/lib -lSDL2
// gcc main.c -o test_sdl2 -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -mmacosx-version-min=15.0

struct Personagem {
    int x, y;                   // posição do personagem na tela
    int velocityX, velocityY;   // velocidade horizontal e vertical
    int onGround;               // indicador de se está no chão ou pulando
    SDL_Texture* textureIdle2;   // textura idle do personagem (imagem)
    SDL_Texture* textureRun;    // textura de corrida do personagem (imagem)
    SDL_Texture* textureJump;   // textura de pulo do personagem (imagem)
    SDL_Texture* currentTexture; // textura atual do personagem
    SDL_RendererFlip flip;      // orientação do personagem (esquerda ou direita)  
    int frameWidth, frameHeight; // largura e altura de cada quadro no spritesheet
    int currentFrame;           // quadro atual da animação
    int totalFramesIdle2;        // número total de quadros da animação idle
    int totalFramesRun;         // número total de quadros da animação de corrida
    int totalFramesJump;        // número total de quadros da animação de pulo
    Uint32 lastFrameTime;       // tempo do último quadro
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

// Função para lidar com entrada de movimento e pulo
void handleInput(struct Personagem* player, int input) {
    if (input == SDLK_w && player->onGround) {
        player->velocityY = -PULO_ALTURA;
        player->currentTexture = player->textureJump; // Trocar para textura de pulo
        player->currentFrame = 0; // Reiniciar a animação de pulo
        player->onGround = 0;
    }
    if (input == SDLK_d) {
        player->velocityX = 10;
        player->flip = SDL_FLIP_NONE; // Olhar para direita
        if (player->onGround) {
            player->currentTexture = player->textureRun; // Trocar para textura de corrida se no chão
        }
    }
    if (input == SDLK_a) {
        player->velocityX = -10;
        player->flip = SDL_FLIP_HORIZONTAL; // Olhar para esquerda
        if (player->onGround) {
            player->currentTexture = player->textureRun; // Trocar para textura de corrida se no chão
        }
    }
}

// Função para parar a animação de corrida ao soltar as teclas de movimento
void handleKeyUp(struct Personagem* player, int input) {
    if (input == SDLK_a || input == SDLK_d) {
        player->velocityX = 0;
        if (player->onGround) {
            player->currentTexture = player->textureIdle2; // Voltar para textura de idle
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
    
    // Carregar diferentes texturas de fundo
    SDL_Texture* backgroundTextureNormal = loadTexture("./assets/map/forest-ground.bmp", renderer);
    SDL_Texture* backgroundTextureFullscreen = loadTexture("./assets/map/forest-groundG.bmp", renderer);

    // Definir groundY como 80% da altura da janela
    int groundY = (int)(windowHeight * 0.8);

    // Carregar texturas para o jogador e configurar dados da animação
    struct Personagem player = {
        10, groundY - 128, 0, 0, 1,
        loadTexture("./assets/player/idle2.bmp", renderer),
        loadTexture("./assets/player/run.bmp", renderer),
        loadTexture("./assets/player/jump.bmp", renderer),
        NULL, SDL_FLIP_NONE, 128, 128, 0, 6, 8, 4, 0
    };
    player.currentTexture = player.textureIdle2;

    while (1) {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);

        SDL_RenderClear(renderer);

        // Escolhe a textura de fundo com base no estado de tela cheia
        SDL_Texture* backgroundTexture = isFullscreen ? backgroundTextureFullscreen : backgroundTextureNormal;

        // Desenha o fundo
        SDL_Rect bgRect = { 0, groundY, windowWidth, windowHeight - groundY };
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect);

        // Atualiza a posição e animação do personagem
        updatePlayerPosition(&player, groundY);
        updateAnimation(&player);
        drawCharacter(&player, renderer);

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(backgroundTextureNormal);
                SDL_DestroyTexture(backgroundTextureFullscreen);
                SDL_DestroyTexture(player.textureIdle2);
                SDL_DestroyTexture(player.textureRun);
                SDL_DestroyTexture(player.textureJump);
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
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_f) {
                isFullscreen = !isFullscreen;
                SDL_SetWindowFullscreen(window, isFullscreen ? SDL_WINDOW_FULLSCREEN : 0);
                if (!isFullscreen) {
                    SDL_SetWindowSize(window, fixedWindowWidth, fixedWindowHeight);
                }            
            }
        }
        SDL_Delay(16);
    }

    SDL_DestroyTexture(backgroundTextureNormal);
    SDL_DestroyTexture(backgroundTextureFullscreen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

}
