# Pitfall: Rise of Dead

## Descrição

**Pitfall: Rise of Dead** é uma releitura do clássico **'Pitfall!'**, agora ambientado em um mundo pós-apocalíptico infestado por zumbis. O jogador deve escapar de hordas de mortos-vivos enquanto enfrenta obstáculos perigosos. O jogo é vencido ao chegar ao abrigo final, onde o jogador estará seguro.

Prepare-se para uma corrida intensa pela sobrevivência!

## Objetivo

- Fugir das hordas de zumbis que o perseguem.
- Superar obstáculos e armadilhas ao longo do caminho.
- Alcançar o abrigo final para garantir a vitória.

## **Tutorial de Compilação**

**1-Baixar dependências:**

- sudo apt update

- sudo apt install build-essential libgl1-mesa-dev libegl1-mesa-dev libopenal-dev libx11-dev libxcursor-dev libxrandr-dev libxi-dev libasound2-dev libvulkan-dev git cmake

**2- Baixar raylib:** 

- sudo apt install libraylib-dev

**3- Comandos para compilação:** 

- gcc main.c -o main -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

  (ou o seguinte - alternativo para Mac)
- gcc main.c -o main -lraylib -lm -lpthread -ldl 

- ./main
