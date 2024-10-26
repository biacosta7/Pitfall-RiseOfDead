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
        player->velocityX = 2;  // velocidade para direita
        player->flip = SDL_FLIP_HORIZONTAL;
    }
    if (input == SDLK_a) {
        player->velocityX = -2; // velocidade para esquerda
        player->flip = SDL_FLIP_NONE;
    }
}

// função para atualizar a posição do personagem
void updatePlayerPosition(struct Personagem* personagem, int groundY, int personagemAltura) {
    personagem->x += personagem->velocityX; // atualiza a posição horizontal

    if (!personagem->onGround) { 
        personagem->velocityY += GRAVIDADE;
        personagem->y += personagem->velocityY;

        if (personagem->y >= groundY - personagemAltura) {
            personagem->y = groundY - personagemAltura;
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

    int windowWidth=800, windowHeight=400;

    SDL_Window* window = SDL_CreateWindow("Pitfall: Rise Of Dead", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    
    SDL_Texture* backgroundTexture = loadTexture("./assets/map/ground-green.bmp", renderer);

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // defini o groundY como 80% da altura da janela
    int groundY = (int)(windowHeight * 0.8);
    int personagemLargura = windowWidth / 10;
    int personagemAltura = windowHeight / 6;

    // carregar texturas para o jogador e zumbis
    struct Personagem player = {10, groundY - personagemAltura, 0, 0, 1, loadTexture("./assets/player/player6.bmp", renderer), SDL_FLIP_HORIZONTAL};
    struct Personagem zumbi_frank = {100, groundY - personagemAltura, 0, 0, 1, loadTexture("./assets/zombie-f/zumbi_f3.bmp", renderer), SDL_FLIP_NONE};
    struct Personagem zumbi_yoda = {300, groundY - personagemAltura, 0, 0, 1, loadTexture("./assets/zombie-y/zumbi_y3.bmp", renderer), SDL_FLIP_NONE};
    struct Personagem zumbi_hairy = {500, groundY - personagemAltura, 0, 0, 1, loadTexture("./assets/zombie-h/zombie-solo.bmp", renderer), SDL_FLIP_NONE};

    while (1) {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        personagemLargura = windowWidth / 10;
        personagemAltura = windowHeight / 6;

        SDL_RenderClear(renderer);

        // desenha o fundo com o tamanho da janela
        SDL_Rect bgRect = { 0, groundY, windowWidth, windowHeight - groundY};
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &bgRect);

        // atualiza e desenha o jogador e zumbis
        updatePlayerPosition(&player, groundY, personagemAltura);
        drawBitmap(&player, personagemLargura, personagemAltura, renderer);
        drawBitmap(&zumbi_frank, personagemLargura, personagemAltura, renderer);
        drawBitmap(&zumbi_yoda, personagemLargura, personagemAltura, renderer);
        drawBitmap(&zumbi_hairy, personagemLargura, personagemAltura, renderer);

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
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_f) {
                isFullscreen = !isFullscreen; // Alterna o estado
                if (isFullscreen) {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                } else {
                    SDL_SetWindowFullscreen(window, 0);
                    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
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
