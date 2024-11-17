#include "raylib.h"

#define RAYLIB_TEXT_UTF8

int main(void) {
    InitWindow(800, 600, "Teste Acentos");
    SetTargetFPS(60);

    Font fontePersonalizada = LoadFont("assets/fonts/alkhemikal.ttf"); // Tente Arial para garantir compatibilidade
    const char *historiaDoJogo = "Em uma sociedade marcada pela decadência, a elite recrutou\n\n" 
                                "uma equipe de cientistas e após anos de pesquisa em um projeto\n\n"
                                "secreto, criou uma substância destinada à imortalidade,\n\n"
                                "acreditando ser a única esperança para a sobrevivência\n\n"
                                "humana. Porém, o experimento saiu do controle, transformando \n\n"
                                "a maioria da população em zumbis. Você é um dos poucos que \n\n"
                                "escaparam de uma tentativa de invasão a essa fortaleza,\n\n"
                                "mas agora, na floresta densa, hordas de zumbis estão por\n\n"
                                "toda parte. Para alcançar a segurança, você deve correr\n\n"
                                "por perigosas áreas infestadas, coletar suprimentos.\n\n"
                                "Sua meta é chegar a um abrigo subterrâneo, onde os últimos\n\n"
                                "cientistas tentam criar uma vacina.\n\n"; // Teste simples com acentos
    Vector2 posHistoria = { 10, 50 };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // Teste com DrawTextEx
        //DrawTextEx(fontePersonalizada, historiaDoJogo, posHistoria, 20, 1, WHITE);

        // Teste com DrawText
        DrawText(historiaDoJogo, 10, 200, 20, WHITE);

        EndDrawing();
    }

    UnloadFont(fontePersonalizada);
    CloseWindow();
    return 0;
}
