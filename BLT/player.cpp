#include "player.h"
#include "def.h"
#include <algorithm>

Player::Player(int startX, int startY, int size) {
    x = startX;
    y = startY;
    width = size;
    height = size;
    stepX = 6.5f;
    isJumping = false;
    velocityY = 0.0f;
    gravity = 0.3f;
    jumpStrength = -9.0f;
    texture = nullptr;
    facingLeft = false;
    jumpSound = nullptr;
    isFallingSoundPlaying = false;
    previousVelocityY = 0.0f; // Initialize previous velocity
}

Player::~Player() {
    // Texture is managed externally
}

void Player::render(SDL_Renderer* renderer) {
    if (texture) {
        SDL_Rect destRect = {x, y - height, width, height};
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
    }
}

void Player::update(std::vector<Platform>& platforms) {

    if (isJumping) {
        velocityY += gravity;
        // Apply movement in small steps to improve collision detection
        float remainingMovement = velocityY;
        int steps = std::max(1, int(std::abs(remainingMovement)));
        float dy = remainingMovement / steps;

        for (int i = 0; i < steps; ++i) {
            y += dy;

            // Check collision with all platforms
            for (auto& platform : platforms) {
                if (checkPlatformCollision(platform)) {
                    break;
                }
            }

            // Break if we're no longer jumping
            if (!isJumping) break;
        }
    }
}

void Player::setJumpSound (Mix_Chunk* sound) {
    jumpSound = sound;
}

void Player::jump() {
    if (!isJumping) {
        isJumping = true;
        velocityY = jumpStrength;

        if(jumpSound) {
            Mix_PlayChannel(-1, jumpSound, 0);
        }
    }
}

void Player::moveRight() {
    x += stepX;
    facingLeft = false;

    // Wrap around screen edges
    if (x > SCREEN_WIDTH) {
        x = -width;
    }
}

void Player::moveLeft() {
    x -= stepX;
    facingLeft = true;

    // Wrap around screen edges
    if (x < -width) {
        x = SCREEN_WIDTH;
    }
}

bool Player::checkPlatformCollision(Platform& platform) {
    // Only check foot collision when falling
    if (velocityY < 0) return false;

    // Skip broken platforms
    if (platform.isBroken()) return false;

    // Create foot rectangle
    const int footHeight = 5;
    SDL_Rect footRect = {
        x,
        y - footHeight,
        width,
        footHeight
    };

    // Get platform rectangle
    SDL_Rect platformRect = platform.getRect();

    // Check intersection
    if (SDL_HasIntersection(&footRect, &platformRect)) {
        y = platformRect.y;
        velocityY = 0;
        isJumping = false;

        // If this is a breakable platform, trigger break
        if (platform.isBreakable()) {
            platform.startBreaking();
        }

        return true;
    }

    return false;
}

void Player::setTexture(SDL_Texture* newTexture) {
    texture = newTexture;
}

void Player::setTextures(SDL_Texture* leftTexture, SDL_Texture* rightTexture) {
    // This method would be used if you want to switch textures based on direction
    // For simplicity, we're only using a single texture in this implementation
    texture = facingLeft ? leftTexture : rightTexture;
}

void Player::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

