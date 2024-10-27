#include <SDL2/SDL.h>
#include <stdio.h>
#define GRAVIDADE 1
#define PULO_ALTURA 17
// gcc -I/usr/local/include/SDL2 ./main.c -o test.exe -L/usr/local/lib -lSDL2

struct Personagem {
    int x, y;                   // posição do personagem na tela
    int velocityX, velocityY;   // velocidade horizontal e vertical
    int onGround;               // indicador de se está no chão ou pulando
    SDL_Texture* texture;       // textura do personagem (imagem)
    SDL_RendererFlip flip;      // orientação do personagem (esquerda ou direita)  
};

// função para carregar uma textura a partir de um arquivo BMP
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

// função para desenhar um bitmap na tela
void drawBitmap(struct Personagem* personagem, int largura, int altura, SDL_Renderer* renderer) {
    SDL_Rect destRect = { personagem->x, personagem->y, largura, altura };
    SDL_RenderCopyEx(renderer, personagem->texture, NULL, &destRect, 0, NULL, personagem->flip);
}

// função para lidar com entrada de movimento e pulo
void handleInput(struct Personagem* player, int input) {
    if (input == SDLK_w && player->onGround) {
        player->velocityY = -PULO_ALTURA;
        player->onGround = 0;
    }
    if (input == SDLK_d) {
        player->velocityX = 2;
        player->flip = SDL_FLIP_HORIZONTAL;
    }
    if (input == SDLK_a) {
        player->velocityX = -2;
        player->flip = SDL_FLIP_NONE;
    }
}

// função para atualizar a posição do personagem
void updatePlayerPosition(struct Personagem* personagem, int groundY) {
    personagem->x += personagem->velocityX;

    if (!personagem->onGround) { 
        personagem->velocityY += GRAVIDADE;
        personagem->y += personagem->velocityY;

        int texturaLargura, texturaAltura;
        SDL_QueryTexture(personagem->texture, NULL, NULL, &texturaLargura, &texturaAltura);

        if (personagem->y >= groundY - texturaAltura) {
            personagem->y = groundY - texturaAltura;
            personagem->onGround = 1;
            personagem->velocityY = 0;
        }
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
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    
    // Carregar diferentes texturas de fundo
    SDL_Texture* backgroundTextureNormal = loadTexture("./assets/map/forest-ground.bmp", renderer);
    SDL_Texture* backgroundTextureFullscreen = loadTexture("./assets/map/forest-groundG.bmp", renderer);

    // defini o groundY como 80% da altura da janela
    int groundY = (int)(windowHeight * 0.8);
    int personagemLargura = windowWidth / 10;
    int personagemAltura = windowHeight / 6;

    // carregar texturas para o jogador e zumbis
    struct Personagem player = {10, groundY - personagemAltura, 0, 0, 1, loadTexture("./assets/player/player6.bmp", renderer), SDL_FLIP_HORIZONTAL};

    while (1) {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        personagemLargura = windowWidth / 10;
        personagemAltura = windowHeight / 6;

        SDL_RenderClear(renderer);

        // Escolhe a textura de fundo com base no estado de tela cheia
        SDL_Texture* backgroundTexture;
        if (isFullscreen) {
            backgroundTexture = backgroundTextureFullscreen;
        } else {
            backgroundTexture = backgroundTextureNormal;
        }

        // desenha o fundo
        SDL_Rect bgRect = { 0, groundY, windowWidth, windowHeight - groundY };
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect);

        updatePlayerPosition(&player, groundY);
        drawBitmap(&player, personagemLargura, personagemAltura, renderer);

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(backgroundTextureNormal);
                SDL_DestroyTexture(backgroundTextureFullscreen);
                SDL_DestroyTexture(player.texture);
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
                if (input == SDLK_a || input == SDLK_d) {
                    player.velocityX = 0;
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_f) {
                isFullscreen = !isFullscreen; // alterna o estado
                if (isFullscreen) {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                } else {
                    SDL_SetWindowFullscreen(window, 0);
                    SDL_SetWindowSize(window, fixedWindowWidth, fixedWindowHeight); // restaura o tamanho fixo da janela
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

    return 0;
}
