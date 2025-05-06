#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "player.h"
#include "platform.h"

class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;

    Player* player;
    PlatformManager* platformManager;
    Mix_Chunk* jumpSound;
    Mix_Chunk* fallSound;

    SDL_Texture* menuTexture;
    SDL_Texture* backgroundTexture;
    SDL_Texture* playerLeftTexture;
    SDL_Texture* playerRightTexture;
    SDL_Texture* platformTexture;
    SDL_Texture* movingPlatformTexture;
    SDL_Texture* breakablePlatformTexture;

    int score;
    int bestScore;
    int cameraThreshold;

    void handleEvents();
    void update();
    void render();
    void loadTextures();
    void loadSounds(); // Add this line
    bool isOnMenu;

public:
    Game();
    ~Game();

    bool init();
    void run();
};

#endif // GAME_H_INCLUDED
