#include "platform.h"
#include "def.h"
#include <algorithm>

// Platform implementation
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

Platform::~Platform() {
    // Texture is managed externally
}

void Platform::render(SDL_Renderer* renderer) {
    // Don't render if platform is broken
    if (broken) return;

    if (texture) {
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    } else {
        // Fallback if no texture
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void Platform::update() {
    // Handle moving platforms
    if (type == PlatformType::MOVING) {
        rect.x += direction * speed;

        // Change direction if hitting screen edge
        if (rect.x <= 0) {
            direction = 1;
        } else if (rect.x + rect.w >= screenWidth) {
            direction = -1;
        }
    }

    // Handle breakable platforms
    if (type == PlatformType::BREAKABLE && breakTimer > 0) {
        breakTimer--;
        // If timer reaches 0, break the platform
        if (breakTimer == 0) {
            broken = true;
        }
    }
}

void Platform::startBreaking() {
    if (type == PlatformType::BREAKABLE && !broken) {
        breakTimer = 15; // Breaks after 15 frames (adjust as needed)
    }
}

// PlatformManager implementation
PlatformManager::PlatformManager(int screenWidth, int screenHeight) {
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    platformWidth = PLATFORM_WIDTH;
    platformHeight = PLATFORM_HEIGHT;

    // Initialize random generator
    std::random_device rd;
    rng = std::mt19937(rd());
    xDist = std::uniform_int_distribution<int>(0, screenWidth - platformWidth);
    typeDist = std::uniform_int_distribution<int>(0, 10); // For platform type randomization

    platformTexture = nullptr;
    movingPlatformTexture = nullptr;
    breakablePlatformTexture = nullptr;

    difficultyLevel = 0;
    platformsPerLevel = 5;  // Start with 5 platforms per level
    basePlatformCount = 15; // Start with 15 platforms
}

PlatformManager::~PlatformManager() {
    // Textures are managed externally
}

void PlatformManager::initialize(int numPlatforms) {
    platforms.clear();

    // Create starting platform under player
    int startX = 50;
    int startY = 60;

    platforms.push_back(Platform(startX, startY, platformWidth, platformHeight, PlatformType::NORMAL));

    // Set texture for the first platform
    if (platformTexture) {
        platforms.back().setTexture(platformTexture);
    }

    // Generate initial platforms
    for (int i = 1; i < numPlatforms; i++) {
        int y = screenHeight - (i * (screenHeight / numPlatforms));
        int x = xDist(rng);

        // Determine platform type (mostly normal at the beginning)
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

        // Set appropriate texture
        if (platformType == PlatformType::MOVING && movingPlatformTexture) {
            platforms.back().setTexture(movingPlatformTexture);
        } else if (platformType == PlatformType::BREAKABLE && breakablePlatformTexture) {
            platforms.back().setTexture(breakablePlatformTexture);
        } else if (platformTexture) {
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

    // Remove broken platforms
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
    // Remove platforms that are below the screen
    platforms.erase(
        std::remove_if(platforms.begin(), platforms.end(),
            [this](const Platform& p) {
                return p.getRect().y > screenHeight;
            }),
        platforms.end()
    );
}

void PlatformManager::updateDifficulty(int score) {
    // Adjust difficulty based on score
    // Every 1000 points, reduce platforms per level
    int newLevel = score / 1000;

    if (newLevel != difficultyLevel) {
        difficultyLevel = newLevel;

        // Gradually reduce platforms per level, but don't go below 2
        platformsPerLevel = std::max(2, 5 - (difficultyLevel / 2));
    }
}

int PlatformManager::getPlatformsToGenerate() const {
    // Calculate how many platforms to generate based on current difficulty
    return platformsPerLevel;
}

void PlatformManager::addNewPlatforms(int numToAdd) {
    if (platforms.empty()) return;

    // Find the highest platform to start adding new ones
    int highestY = screenHeight;
    for (const auto& platform : platforms) {
        if (platform.getRect().y < highestY) {
            highestY = platform.getRect().y;
        }
    }

    // Vertical gap between platforms - increases with difficulty level
    int verticalGap = MAX_JUMP_HEIGHT * 0.75 * (1.0f + (difficultyLevel * 0.1f));
    int currentY = highestY;

    // Add new platforms
    for (int i = 0; i < numToAdd; i++) {
        // Decrease Y position to create higher platforms
        currentY -= verticalGap;

        // Choose a random X position
        int newX = xDist(rng);

        // Determine platform type based on difficulty
        PlatformType platformType = PlatformType::NORMAL;

        // First platform shouldn't be breakable or moving to make it easier to jump on
        if (i > 0) {
            int randVal = rand() % 100;

            // Calculate chances based on difficulty level
            int movingChance = 15 + (difficultyLevel * 2); // Starts at 15%, increases by 2% per level
            int breakableChance = 10 + (difficultyLevel * 3); // Starts at 10%, increases by 3% per level

            // Cap chances
            movingChance = std::min(movingChance, 30);
            breakableChance = std::min(breakableChance, 40);

            if (randVal < breakableChance) {
                platformType = PlatformType::BREAKABLE;
            } else if (randVal < breakableChance + movingChance) {
                platformType = PlatformType::MOVING;
            }
        }

        // Create new platform
        platforms.push_back(Platform(newX, currentY, platformWidth, platformHeight, platformType));

        // Set appropriate texture
        if (platformType == PlatformType::MOVING && movingPlatformTexture) {
            platforms.back().setTexture(movingPlatformTexture);
        } else if (platformType == PlatformType::BREAKABLE && breakablePlatformTexture) {
            platforms.back().setTexture(breakablePlatformTexture);
        } else if (platformTexture) {
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

    // Update existing platforms
    for (auto& platform : platforms) {
        if (platform.getType() == PlatformType::MOVING && movingPlatformTexture) {
            platform.setTexture(movingPlatformTexture);
        } else if (platform.getType() == PlatformType::BREAKABLE && breakablePlatformTexture) {
            platform.setTexture(breakablePlatformTexture);
        } else {
            platform.setTexture(platformTexture);
        }
    }
}
