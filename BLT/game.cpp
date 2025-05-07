#include "game.h"
#include "def.h"
#include "graphics.h"
#include <iostream>
#include <algorithm>
#include <SDL_ttf.h>
#include <fstream>

Game::Game() {
    window = nullptr;
    renderer = nullptr;
    isRunning = false;
    player = nullptr;
    platformManager = nullptr;

    menuTexture = nullptr;
    backgroundTexture = nullptr;
    playerLeftTexture = nullptr;
    playerRightTexture = nullptr;
    platformTexture = nullptr;
    movingPlatformTexture = nullptr;
    breakablePlatformTexture = nullptr;

    jumpSound = NULL;
    font = nullptr;

    score = 0;
    bestScore = 0;
    cameraThreshold = 300;

    isOnMenu = true;
    isMuted = false;
    isGameOver = false;
}

Game::~Game() {
    if (playerLeftTexture) SDL_DestroyTexture(playerLeftTexture);
    if (playerRightTexture) SDL_DestroyTexture(playerRightTexture);
    if (platformTexture) SDL_DestroyTexture(platformTexture);
    if (movingPlatformTexture) SDL_DestroyTexture(movingPlatformTexture);
    if (breakablePlatformTexture) SDL_DestroyTexture(breakablePlatformTexture);
    if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
    if (jumpSound) Mix_FreeChunk(jumpSound);
    if (font) TTF_CloseFont(font);

    TTF_Quit();

    delete player;
    delete platformManager;

    quitSDL(window, renderer);
}

bool Game::init() {
    window = initSDL(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    renderer = createRenderer(window);

    if (!window || !renderer) return false;

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    if (!initAudio()) {
        std::cerr << "Failed to initialize audio!" << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }
    font = TTF_OpenFont("./font/font.ttf", 30);
    if (!font) {
        std::cerr << "Failed to load font! TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }

    loadTextures();
    loadSounds();

    player = new Player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 80);
    player->setTexture(playerLeftTexture);
    player->setJumpSound(jumpSound);

    platformManager = new PlatformManager(SCREEN_WIDTH, SCREEN_HEIGHT);
    platformManager->setTextures(platformTexture, movingPlatformTexture, breakablePlatformTexture);
    platformManager->initialize(10);

    isRunning = true;
    loadBestScore();
    return true;
}

void Game::loadTextures() {
    menuTexture = loadTexture("./images/menu.png", renderer);
    backgroundTexture = loadTexture("./images/background .png", renderer);
    playerLeftTexture = loadTexture("./images/playerleft.png", renderer);
    playerRightTexture = loadTexture("./images/playerright.png", renderer);
    platformTexture = loadTexture("./images/platform.png", renderer);
    movingPlatformTexture = loadTexture("./images/movingplatform.png", renderer);
    breakablePlatformTexture = loadTexture("./images/brown_platform_breaking_.png", renderer);
}

void Game::loadSounds() {
    jumpSound = Mix_LoadWAV("./sound/jumpSound.mp3");
    if (!jumpSound) {
        std::cerr << "Failed to load jump sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }


}

void Game::handleEvents() {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
        }
        else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                isRunning = false;
            }
            if (e.key.keysym.sym == SDLK_m) {
                isMuted = !isMuted;
                int volume = isMuted ? 0 : MIX_MAX_VOLUME;
                Mix_Volume(-1, volume);
                Mix_VolumeChunk(jumpSound, volume);
            }
            if (isOnMenu) {
                isOnMenu = false;
                return;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (isOnMenu) {
                isOnMenu = false;
                return;
            }
        }
    }

    if (isOnMenu) return;

    const Uint8* keystates = SDL_GetKeyboardState(NULL);

    if (!player->getIsJumping()) {
        player->jump();
    }

    if (keystates[SDL_SCANCODE_RIGHT]) {
        player->moveRight();
        player->setTexture(playerRightTexture);
    }

    if (keystates[SDL_SCANCODE_LEFT]) {
        player->moveLeft();
        player->setTexture(playerLeftTexture);
    }
}

void Game::update() {
    if (isOnMenu || isGameOver) return;

    player->update(platformManager->getPlatforms());
    platformManager->update();
    platformManager->updateDifficulty(score);

    if (player->getY() < cameraThreshold) {
        int scrollAmount = cameraThreshold - player->getY();
        player->setPosition(player->getX(), cameraThreshold);
        platformManager->scrollPlatforms(scrollAmount);
        score += scrollAmount;
        bestScore = std::max(score, bestScore);
        platformManager->removeBottomPlatforms();
        int platformsToAdd = platformManager->getPlatformsToGenerate();
        platformManager->addNewPlatforms(platformsToAdd);
    }

    if (player->getY() > SCREEN_HEIGHT) {
        saveBestScore();
        handleGameOverScreen();
    }
}

void Game::render() {
    SDL_RenderClear(renderer);

    if (isOnMenu) {
        SDL_RenderCopy(renderer, menuTexture, NULL, NULL);
        Uint32 time = SDL_GetTicks();
        if(time / 400 % 2 == 0) {
            displayText("Press any key", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 20);
            displayText("to play", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 50);
        }
        SDL_RenderPresent(renderer);
        return;
    }

    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    }

    platformManager->render(renderer);
    player->render(renderer);
    displayText("Score: " + std::to_string(score), 280, 10);

    std::string soundStatus = isMuted ? "Sound: Off" : "Sound: On";
    displayText(soundStatus, 10, 10);

    SDL_RenderPresent(renderer);

    if (isGameOver) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    displayText("Game Over!", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 40);
    displayText("Press any key to retry", SCREEN_WIDTH / 2 - 130, SCREEN_HEIGHT / 2);
    displayText("Best Score: " + std::to_string(bestScore), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 40);

    SDL_RenderPresent(renderer);
    SDL_Delay(100);
    return;
}

}

void Game::run() {
    while (isRunning) {
        handleEvents();
        update();
        render();
        SDL_Delay(0);
    }
}

void Game::displayText(const std::string& text, int x, int y, SDL_Color color) {
    if (!font) return;
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }
    SDL_Rect renderQuad = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Game::saveBestScore() {
    std::ofstream outFile("highscore.txt");
    if (outFile.is_open()) {
        outFile << bestScore;
        outFile.close();
    } else {
        std::cerr << "Failed to save high score!" << std::endl;
    }
}

void Game::loadBestScore() {
    std::ifstream inFile("highscore.txt");
    if (inFile.is_open()) {
        inFile >> bestScore;
        inFile.close();
    } else {
        bestScore = 0;
    }
}

void Game::handleGameOverScreen() {
    SDL_Event e;
    bool waiting = true;

    while (waiting) {
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        displayText("Game Over!", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 80);
        displayText("Press R to retry", SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 - 40);
        displayText("Best Score: " + std::to_string(bestScore), SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 );

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                isRunning = false;
                waiting = false;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                waiting = false;
            }
        }

        SDL_Delay(0);
    }

    // Reset trạng thái game
    isGameOver = false;
    score = 0;
    player->setPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    platformManager->initialize(15);
}

