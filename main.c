#include <SDL2/SDL.h>
#include <stdio.h>
#define GRAVITY 2
#define GROUND_Y 150
#define JUMP_HEIGHT 15

// Para rodar: gcc -I./libs/include -I/usr/local/include/SDL2 ./main.c ./libs/src/*.c -o test.exe -L/usr/local/lib -lSDL2

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
void drawBitmap(int x, int y, int width, int height, SDL_Texture* texture, SDL_Renderer* renderer) {
    SDL_Rect destRect = { x, y, width, height };  // largura e altura da textura
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
}

int speed = 10; // velocidade
int onGround = 0;
int velocityY = 0;
int velocityX = 10; // Velocidade horizontal

void handleInput(int input) {
    if (input == SDLK_w && onGround) {  
        velocityY = -JUMP_HEIGHT;  // Inicia o pulo
        onGround = 0;
    }
    if (input == SDLK_d) { 
        velocityX = speed;   // Move para direita
    }
    if (input == SDLK_a) {  
        velocityX = -speed;  // Move para esquerda
    }
}
void updatePlayerPosition(int *x,int *y){
    // Movimento horizontal
    *x += velocityX;

    if(!onGround){
        velocityY += GRAVITY; //aplica gravidade
        *y += velocityY; //atualiza posicao vertical
        if(*y >= 220){
            *y = 220;
            onGround = 1;
            velocityY = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    int x = 10, y = 210;  // posição inicial do personagem
    int input;

    // inicializa o SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pitfall: Rise Of Dead", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    
    SDL_Texture* backgroundTexture = loadTexture("./assets/map/layers/Layer_0002_7.bmp", renderer);

    const char* bmpFilePath = "./assets/zombie-solo.bmp";
    SDL_Texture* characterTexture = loadTexture(bmpFilePath, renderer);

    // Definindo a largura e altura do solo
    int groundWidth = 800; // largura total da janela
    int groundHeight = 450; // altura de verdade solo (nao é ao contrário)
    while (1) {
        // limpa a tela
        SDL_RenderClear(renderer);

        // desenha o solo na parte inferior
        drawBitmap(0, GROUND_Y, groundWidth, groundHeight, backgroundTexture, renderer); // desenha o solo na parte inferior
        updatePlayerPosition(&x,&y);
        // desenha o personagem com o bitmap na nova posição
        drawBitmap(x, y, 64, 64, characterTexture, renderer);
        SDL_RenderPresent(renderer);  // atualiza a tela

        // processa eventos
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
                
                // Chama a função para pulo e movimento inicial
                handleInput(input);
            }
            if (event.type == SDL_KEYUP) {
                input = event.key.keysym.sym;
                
                // Para o movimento horizontal quando as teclas são soltas
                if (input == SDLK_a || input == SDLK_d) {
                    velocityX = 0;
                }
            }
        }
        SDL_Delay(16);  // limita a taxa de atualização (aprox. 60 fps)
    }

    // libera 
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(characterTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}