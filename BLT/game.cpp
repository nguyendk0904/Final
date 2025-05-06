#include "game.h"
#include "def.h"
#include "graphics.h"
#include <iostream>
#include <algorithm>

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
    fallSound = NULL;

    score = 0;
    bestScore = 0;
    cameraThreshold = 300;

    isOnMenu = true;
}

Game::~Game() {
    if (playerLeftTexture) SDL_DestroyTexture(playerLeftTexture);
    if (playerRightTexture) SDL_DestroyTexture(playerRightTexture);
    if (platformTexture) SDL_DestroyTexture(platformTexture);
    if (movingPlatformTexture) SDL_DestroyTexture(movingPlatformTexture);
    if (breakablePlatformTexture) SDL_DestroyTexture(breakablePlatformTexture);
    if (backgroundTexture) SDL_DestroyTexture(backgroundTexture);
    if (jumpSound) Mix_FreeChunk(jumpSound);
    if (fallSound) Mix_FreeChunk(fallSound);

    delete player;
    delete platformManager;

    quitSDL(window, renderer);
}

bool Game::init() {
    // Initialize SDL
    window = initSDL(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    renderer = createRenderer(window);

    if (!window || !renderer) {
        return false;
    }

    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_mixer
    if (!initAudio()) {
        std::cerr << "Failed to initialize audio!" << std::endl;
        return false;
    }

    // Load textures
    loadTextures();

    loadSounds();

    // Create player
    player = new Player(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 80);
    player->setTexture(playerLeftTexture);
    player->setJumpSound(jumpSound); // Set the jump sound for the player
    player->setFallSound(fallSound);

    // Create platform manager
    platformManager = new PlatformManager(SCREEN_WIDTH, SCREEN_HEIGHT);
    platformManager->setTextures(platformTexture, movingPlatformTexture, breakablePlatformTexture);
    platformManager->initialize(15);

    isRunning = true;
    return true;
}

void Game::loadTextures() {
    menuTexture = loadTexture("./images/menu.png", renderer);
    backgroundTexture = loadTexture("./images/background .png", renderer);
    playerLeftTexture = loadTexture("./images/playerleft.png", renderer);
    playerRightTexture = loadTexture("./images/playerright.png", renderer);
    platformTexture = loadTexture("./images/platform.png", renderer);
    movingPlatformTexture = loadTexture("./images/movingplatform.png", renderer);
    breakablePlatformTexture = loadTexture("./images/breakplatform.png", renderer);
}

void Game::loadSounds() {
    jumpSound = Mix_LoadWAV("./sound/jumpSound.mp3");
    if (!jumpSound) {
        std::cerr << "Failed to load jump sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }

    fallSound = Mix_LoadWAV(".sound/fallSound.mp3");
    if (!jumpSound) {
        std::cerr << "Failed to load fall sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}


void Game::handleEvents() {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                isRunning = false;
            }

            if (isOnMenu) {
                isOnMenu = false; // Thoát menu khi nhấn phím bất kỳ
                return;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (isOnMenu) {
                isOnMenu = false; // Thoát menu khi click chuột
                return;
            }
        }
    }

    if (isOnMenu) return; // Không xử lý game nếu đang ở menu

    // Handle keyboard state
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
    if(isOnMenu) return;
    // Update player
    player->update(platformManager->getPlatforms());

    // Update platforms
    platformManager->update();

    // Update difficulty based on score
    platformManager->updateDifficulty(score);

    // Camera scroll
    if (player->getY() < cameraThreshold) {
        int scrollAmount = cameraThreshold - player->getY();
        player->setPosition(player->getX(), cameraThreshold);

        platformManager->scrollPlatforms(scrollAmount);

        // Update score (1 point per pixel scrolled)
        score += scrollAmount;
        bestScore = std::max(score, bestScore);

        // Remove platforms that fell off the bottom
        platformManager->removeBottomPlatforms();

        // Add new platforms based on current difficulty
        int platformsToAdd = platformManager->getPlatformsToGenerate();
        platformManager->addNewPlatforms(platformsToAdd);
    }
    // Game over condition - also play falling sound at max volume when player falls off the screen
    if (player->getY() > SCREEN_HEIGHT - 100 && player->getY() <= SCREEN_HEIGHT && fallSound) {
        // Play falling sound at higher volume for dramatic effect as player is about to die
        Mix_VolumeChunk(fallSound, MIX_MAX_VOLUME);
        Mix_PlayChannel(-1, fallSound, 0);
    }

    // Game over condition
    if (player->getY() > SCREEN_HEIGHT) {
        std::cout << "Game Over! Score: " << score << std::endl;
        std::cout << "Best Score: " << bestScore << std::endl;

        // Reset game
        score = 0;
        player->setPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        platformManager->initialize(15);
    }
}

void Game::render() {
    SDL_RenderClear(renderer);

    if (isOnMenu) {
        SDL_RenderCopy(renderer, menuTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
        return;
    }

    // Render background
    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    }

    // Render platforms
    platformManager->render(renderer);

    // Render player
    player->render(renderer);

    // TODO: Render score

    SDL_RenderPresent(renderer);
}


void Game::run() {
    while (isRunning) {
        handleEvents();
        update();
        render();

        std::cout << score << "\n";
        SDL_Delay(0); // Giới hạn 60 FPS
    }
}

