#include <SDL2/SDL.h>
#include <stdio.h>
#include "screen.h"
#include "keyboard.h"

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

// Função para desenhar um bitmap na tela
void drawBitmap(int x, int y, SDL_Texture* texture, SDL_Renderer* renderer) {
    SDL_Rect destRect = { x, y, 64, 64 };  // largura e altura da textura
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
}

int speed = 5; // velocidade

void movePlayer(int *x, int *y, char input) {
    if (input == 'w') {  // Cima
        (*y) -= speed; // Mova para cima
    } else if (input == 's') {  // Baixo
        (*y) += speed; // Mova para baixo
    } else if (input == 'a') {  // Esquerda
        (*x) -= speed; // Mova para a esquerda
    } else if (input == 'd') {  // Direita
        (*x) += speed; // Mova para a direita
    }
}

int main() {
    int x = 100, y = 100;  // Posição inicial do personagem
    int input;

    // Inicializa o SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("PITFALL!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    
    const char* bmpFilePath = "./assets/zombie-solo.bmp"; // Caminho do BMP
    SDL_Texture* characterTexture = loadTexture(bmpFilePath, renderer);

    screenInit(1); // o 1 indica que vou desenhar bordas
    keyboardInit();    

    while (1) {
        // Limpa a tela
        SDL_RenderClear(renderer);
        
        // Desenha o personagem com o bitmap na nova posição
        drawBitmap(x, y, characterTexture, renderer);
        SDL_RenderPresent(renderer);  // Atualiza a tela

        // Processa eventos
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(characterTexture);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
            if (event.type == SDL_KEYDOWN) {
                input = event.key.keysym.sym;

                // Chama a função para mover o personagem
                movePlayer(&x, &y, input);
            }
        }

        // Limita a taxa de atualização
        SDL_Delay(16);  // Aproximadamente 60 FPS
    }

    // Libera recursos
    SDL_DestroyTexture(characterTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
