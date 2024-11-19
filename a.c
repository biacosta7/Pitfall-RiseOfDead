void UpdateFinalPhase(void) {
    switch(currentPhase) {
        case FINAL_PHASE:
            // Spawn final boss
            if (!bossSpawned) {
                SpawnBoss();
                bossSpawned = true;
            }
            
            // Change background or environment
            DrawFinalPhaseBackground();
            
            // Add special effects
            if (bossSpawned) {
                UpdateBossLogic();
                DrawBossHealthBar();
            }
            
            // Maybe change music
            if (!finalMusicStarted) {
                PlayFinalPhaseMusic();
                finalMusicStarted = true;
            }
            
            // Add special obstacles or challenges
            UpdateFinalPhaseObstacles();
            break;
            
        default:
            // Regular game phase logic
            break;
    }
}

void DrawFinalPhase(void) {
    if (currentPhase == FINAL_PHASE) {
        // Maybe darken the background
        DrawRectangle(0, 0, screenWidth, screenHeight, 
                     ColorAlpha(BLACK, 0.3f));
        
        // Add visual effects
        DrawParticleEffects();
        
        // Draw final boss
        if (bossSpawned) {
            DrawBoss();
        }
        
        // Draw UI elements specific to final phase
        DrawText("FINAL BATTLE", screenWidth/2 - 100, 50, 20, RED);
        
        // Draw special indicators or warnings
        if (bossHealth < bossMaxHealth * 0.3f) {
            DrawText("BOSS ENRAGED!", screenWidth/2 - 80, 80, 20, RED);
        }
    }
}

void CheckFinalPhaseCompletion(void) {
    if (currentPhase == FINAL_PHASE) {
        // Victory condition
        if (bossDefeated) {
            gameState = VICTORY;
            PlayVictoryMusic();
            // Show victory screen
        }
        
        // Defeat condition
        if (player.lives <= 0) {
            gameState = GAME_OVER;
            PlayDefeatMusic();
            // Show game over screen
        }
    }
}

while (!WindowShouldClose()) {
    UpdateGame();
    
    if (currentPhase == FINAL_PHASE) {
        UpdateFinalPhase();
    }
    
    BeginDrawing();
        ClearBackground(BLACK);
        DrawGame();
        
        if (currentPhase == FINAL_PHASE) {
            DrawFinalPhase();
        }
        
        CheckFinalPhaseCompletion();
    EndDrawing();
}
