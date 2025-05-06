#include "platform.h"
#include "def.h"
#include <algorithm>

Platform::Platform(int x, int y, int width, int height, PlatformType platformType) {
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
    type = platformType;
    speed = 3.5f;
    direction = 1;
    screenWidth = SCREEN_WIDTH;
    texture = nullptr;
    broken = false;
    breakTimer = 0;
}

Platform::~Platform() {}

void Platform::render(SDL_Renderer* renderer) {

    if (broken) return;

    if (texture) {
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }

    else {
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void Platform::update() {
    if (type == PlatformType::MOVING) {
        rect.x += direction * speed;

        if (rect.x <= 0) {

            direction = 1;

        }

        else if (rect.x + rect.w >= screenWidth) {

            direction = -1;

        }
    }

    if (type == PlatformType::BREAKABLE && breakTimer > 0) {

        breakTimer--;

        if (breakTimer == 0) {
            broken = true;
        }
    }
}

void Platform::startBreaking() {
    if (type == PlatformType::BREAKABLE && !broken) {
        breakTimer = 15;
    }
}

PlatformManager::PlatformManager(int screenWidth, int screenHeight) {
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    platformWidth = PLATFORM_WIDTH;
    platformHeight = PLATFORM_HEIGHT;

    std::random_device rd;
    rng = std::mt19937(rd());
    xDist = std::uniform_int_distribution<int>(0, screenWidth - platformWidth);
    typeDist = std::uniform_int_distribution<int>(0, 10);

    platformTexture = nullptr;
    movingPlatformTexture = nullptr;
    breakablePlatformTexture = nullptr;

    difficultyLevel = 0;
    platformsPerLevel = 5;
    basePlatformCount = 15;
}

PlatformManager::~PlatformManager() {}

void PlatformManager::initialize(int numPlatforms) {
    platforms.clear();

    int startX = 50;
    int startY = 60;

    platforms.push_back(Platform(startX, startY, platformWidth, platformHeight, PlatformType::NORMAL));

    if (platformTexture) {
        platforms.back().setTexture(platformTexture);
    }

    for (int i = 1; i < numPlatforms; i++) {
        int y = screenHeight - (i * (screenHeight / numPlatforms));
        int x = xDist(rng);

        int randValue = typeDist(rng);
        PlatformType platformType;

        switch (randValue) {
        case 1:
            platformType = PlatformType::NORMAL;
            break;
        case 0:
            platformType = PlatformType::MOVING;
            break;
        case 2:
            platformType = PlatformType::BREAKABLE;
            break;
    }
        platforms.push_back(Platform(x, y, platformWidth, platformHeight, platformType));

        if (platformType == PlatformType::MOVING && movingPlatformTexture) {
            platforms.back().setTexture(movingPlatformTexture);
        }

        else if (platformType == PlatformType::BREAKABLE && breakablePlatformTexture) {
            platforms.back().setTexture(breakablePlatformTexture);
        }

        else if (platformTexture) {
            platforms.back().setTexture(platformTexture);
        }

        platforms.back().setScreenWidth(screenWidth);
    }
}

void PlatformManager::render(SDL_Renderer* renderer) {
    for (auto& platform : platforms) {
        platform.render(renderer);
    }
}

void PlatformManager::update() {
    for (auto& platform : platforms) {
        platform.update();
    }

    platforms.erase(
        std::remove_if(platforms.begin(), platforms.end(),
            [](const Platform& p) {
                return p.isBroken();
            }),
        platforms.end()
    );
}

void PlatformManager::scrollPlatforms(float scrollAmount) {
    for (auto& platform : platforms) {
        platform.setY(platform.getRect().y + scrollAmount);
    }
}

void PlatformManager::removeBottomPlatforms() {
    platforms.erase(
        std::remove_if(platforms.begin(), platforms.end(),
            [this](const Platform& p) {
                return p.getRect().y > screenHeight;
            }),
        platforms.end()
    );
}

void PlatformManager::updateDifficulty(int score) {
    int newLevel = score / 1000;

    if (newLevel != difficultyLevel) {
        difficultyLevel = newLevel;

        platformsPerLevel = std::max(2, 5 - (difficultyLevel / 2));
    }
}

int PlatformManager::getPlatformsToGenerate() const {
    return platformsPerLevel;
}

void PlatformManager::addNewPlatforms(int numToAdd) {
    if (platforms.empty()) return;

    int highestY = screenHeight;
    for (const auto& platform : platforms) {
        if (platform.getRect().y < highestY) {
            highestY = platform.getRect().y;
        }
    }

    int verticalGap = MAX_JUMP_HEIGHT * 0.75 * (1.0f + (difficultyLevel * 0.1f));
    int currentY = highestY;

    for (int i = 0; i < numToAdd; i++) {
        currentY -= verticalGap;

        int newX = xDist(rng);

        PlatformType platformType = PlatformType::NORMAL;

        if (i > 0) {
            int randVal = rand() % 100;

            int movingChance = 15 + (difficultyLevel * 3);
            int breakableChance = 10 + (difficultyLevel * 10);

            movingChance = std::min(movingChance, 30);
            breakableChance = std::min(breakableChance, 40);

            if (randVal < breakableChance) {
                platformType = PlatformType::BREAKABLE;
            } else if (randVal < breakableChance + movingChance) {
                platformType = PlatformType::MOVING;
            }
        }

        platforms.push_back(Platform(newX, currentY, platformWidth, platformHeight, platformType));

        if (platformType == PlatformType::MOVING && movingPlatformTexture) {
            platforms.back().setTexture(movingPlatformTexture);
        }

        else if (platformType == PlatformType::BREAKABLE && breakablePlatformTexture) {
            platforms.back().setTexture(breakablePlatformTexture);
        }

        else if (platformTexture) {
            platforms.back().setTexture(platformTexture);
        }

        platforms.back().setScreenWidth(screenWidth);
    }
}

bool PlatformManager::isOverlapping(int x, int y) const {
    for (const auto& platform : platforms) {
        SDL_Rect rect = platform.getRect();
        int dx = std::abs(rect.x - x);
        int dy = std::abs(rect.y - y);

        if (dx < platformWidth - MIN_X_GAP && dy < MIN_Y_GAP) {
            return true;
        }
    }
    return false;
}

void PlatformManager::setTextures(SDL_Texture* normalTexture, SDL_Texture* movingTexture, SDL_Texture* breakableTexture) {
    platformTexture = normalTexture;
    movingPlatformTexture = movingTexture ? movingTexture : normalTexture;
    breakablePlatformTexture = breakableTexture ? breakableTexture : normalTexture;

    for (auto& platform : platforms) {
        if (platform.getType() == PlatformType::MOVING && movingPlatformTexture) {
            platform.setTexture(movingPlatformTexture);
        }

        else if (platform.getType() == PlatformType::BREAKABLE && breakablePlatformTexture) {
            platform.setTexture(breakablePlatformTexture);
        }

        else {
            platform.setTexture(platformTexture);
        }
    }
}
