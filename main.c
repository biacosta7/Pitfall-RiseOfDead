#include "screen.h"
#include "keyboard.h"
#include <stdio.h>

// Função para lidar com o movimento do personagem
void movePlayer(int *x, int *y, char input) {
    // Mover o personagem com base na tecla pressionada
    if (input == 'w') {  // cima
        (*y)--;
    } else if (input == 's') {  //baixo
        (*y)++;
    } else if (input == 'a') {  //esquerda
        (*x)--;
    } else if (input == 'd') {  //direita
        (*x)++;
    }

    if (*x < SCRSTARTX) *x = SCRSTARTX;
    if (*x > SCRENDX) *x = SCRENDX;
    if (*y < SCRSTARTY) *y = SCRSTARTY;
    if (*y > SCRENDY) *y = SCRENDY;
}

// No main(), a lógica permanece a mesma
int main() {
    int x = 10, y = 10;  // Posição inicial do personagem
    int input;

    screenClear();   
    screenHideCursor(); 
    keyboardInit();    
    screenInit(1); //o 1 indica que vou desenhar bordas
    screenGotoxy(30, 10);
    printf("PITFALL!");
    screenUpdate();
    while (1) {
        screenGotoxy(x, y);  // Posiciona o cursor na posição atual
        printf("{¬ºཀ°}¬");  // Desenha o personagem na nova posição
        screenUpdate();  // Atualiza a tela imediatamente para mostrar o novo desenho

        if (keyhit()) {  // Verifica se alguma tecla foi pressionada
            input = readch();  // Lê o caractere da tecla pressionada

            // Limpa o personagem da posição anterior
            screenGotoxy(x, y);
            printf(" ");

            if (input == 'q') {  // Tecla 'q' para sair do jogo
                break;
            }

            // Chama a função para mover o personagem
            movePlayer(&x, &y, input);
        }
    }

    keyboardDestroy();   // Destroi o estado do teclado
    screenShowCursor();  // Mostra o cursor novamente ao sair do jogo
    screenClear();       // Limpa a tela ao sair

    return 0;
}