#include <SDL2/SDL.h>
#include <stdio.h>
#define GRAVITY 1
#define GROUND_Y 150
#define JUMP_HEIGHT 17

// gcc -I/usr/local/include/SDL2 ./main.c -o test.exe -L/usr/local/lib -lSDL2

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

// função para desenhar um bitmap na tela com possibilidade de espelhamento
void drawBitmap(int x, int y, int width, int height, SDL_Texture* texture, SDL_Renderer* renderer, SDL_RendererFlip flip) {
    SDL_Rect destRect = { x, y, width, height };
    SDL_RenderCopyEx(renderer, texture, NULL, &destRect, 0, NULL, flip);
}

int speed = 2; // velocidade
int onGround = 0;
int velocityY = 0;
int velocityX = 0;
SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL; // Controle para espelhamento horizontal

// função para lidar com entrada de movimento e pulo
void handleInput(int input) {
    if (input == SDLK_w && onGround) {
        velocityY = -JUMP_HEIGHT;
        onGround = 0;
    }
    if (input == SDLK_d) {
        velocityX = speed;
        flip = SDL_FLIP_HORIZONTAL; // Direção para a esquerda
    }
    if (input == SDLK_a) {
        velocityX = -speed;
        flip = SDL_FLIP_NONE ; // Direção para a direita
    }
}

// função para atualizar a posição do jogador
void updatePlayerPosition(int *x, int *y) {
    *x += velocityX;

    if (!onGround) {
        velocityY += GRAVITY;
        *y += velocityY;
        if (*y >= 220) {
            *y = 220;
            onGround = 1;
            velocityY = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    int x = 10, y = 210;
    int input;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pitfall: Rise Of Dead", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Texture* backgroundTexture = loadTexture("./assets/map/layers/Layer_0002_7.bmp", renderer);
    SDL_Texture* characterTexture = loadTexture("./assets/player/player6.bmp", renderer);

    int groundWidth = 800;
    int groundHeight = 450;

    while (1) {
        SDL_RenderClear(renderer);

        drawBitmap(0, GROUND_Y, groundWidth, groundHeight, backgroundTexture, renderer, SDL_FLIP_NONE);
        updatePlayerPosition(&x, &y);
        drawBitmap(x, y, 64, 64, characterTexture, renderer, flip); // Desenha o personagem com espelhamento
        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(backgroundTexture);
                SDL_DestroyTexture(characterTexture);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
            if (event.type == SDL_KEYDOWN) {
                input = event.key.keysym.sym;
                handleInput(input);
            }
            if (event.type == SDL_KEYUP) {
                input = event.key.keysym.sym;
                if (input == SDLK_a || input == SDLK_d) {
                    velocityX = 0;
                }
            }
        }
        SDL_Delay(16);
    }

    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(characterTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
