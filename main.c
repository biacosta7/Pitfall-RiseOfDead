#include <SDL2/SDL.h>
#include <stdio.h>
#define GRAVIDADE 1
#define GROUND_Y 150
#define PULO_ALTURA 17

// gcc -I/usr/local/include/SDL2 ./main.c -o test.exe -L/usr/local/lib -lSDL2

struct Personagem {
    int x, y;                   // Posição do personagem na tela
    int velocityX, velocityY;   // Velocidade horizontal e vertical
    int onGround;               // Indicador de se está no chão ou pulando
    SDL_Texture* texture;       // Textura do personagem (imagem)
    SDL_RendererFlip flip;      // Orientação do personagem (esquerda ou direita)
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
        player->velocityX = 2;  // Velocidade para direita
        player->flip = SDL_FLIP_HORIZONTAL;
    }
    if (input == SDLK_a) {
        player->velocityX = -2; // Velocidade para esquerda
        player->flip = SDL_FLIP_NONE;
    }
}

// função para atualizar a posição do personagem
void updatePlayerPosition(struct Personagem* personagem) {
    personagem->x += personagem->velocityX;

    if (!personagem->onGround) {
        personagem->velocityY += GRAVIDADE;
        personagem->y += personagem->velocityY;
        if (personagem->y >= 220) {
            personagem->y = 220;
            personagem->onGround = 1;
            personagem->velocityY = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    int input;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pitfall: Rise Of Dead", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Texture* backgroundTexture = loadTexture("./assets/map/ground-gray.bmp", renderer);
    // Carregar texturas para o jogador e zumbis
    struct Personagem player = {10, 210, 0, 0, 1, loadTexture("./assets/player/player6.bmp", renderer), SDL_FLIP_HORIZONTAL};
    struct Personagem zumbi_frank = {100, 220, 0, 0, 1, loadTexture("./assets/zombie-f/zumbi_f3.bmp", renderer), SDL_FLIP_NONE};
    struct Personagem zumbi_yoda = {300, 220, 0, 0, 1, loadTexture("./assets/zombie-y/zumbi_y3.bmp", renderer), SDL_FLIP_NONE};
    struct Personagem zumbi_hairy = {500, 220, 0, 0, 1, loadTexture("./assets/zombie-h/zombie-solo.bmp", renderer), SDL_FLIP_NONE};

    while (1) {
        SDL_RenderClear(renderer);

        // Desenha o fundo
        SDL_Rect bgRect = { 0, GROUND_Y, 800, 450 };
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect);

        // Atualiza e desenha o jogador
        updatePlayerPosition(&player);
        drawBitmap(&player, 64, 64, renderer);

        // Atualiza e desenha zumbis
        drawBitmap(&zumbi_frank, 64, 64, renderer);
        drawBitmap(&zumbi_yoda, 64, 64, renderer);
        drawBitmap(&zumbi_hairy, 64, 64, renderer);

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(backgroundTexture);
                SDL_DestroyTexture(player.texture);
                SDL_DestroyTexture(zumbi_frank.texture);
                SDL_DestroyTexture(zumbi_yoda.texture);
                SDL_DestroyTexture(zumbi_hairy.texture);
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
        }
        SDL_Delay(16);
    }

    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
