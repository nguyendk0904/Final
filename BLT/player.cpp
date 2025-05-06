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
    previousVelocityY = 0.0f;
}

Player::~Player() {}

void Player::render(SDL_Renderer* renderer) {
    if (texture) {
        SDL_Rect destRect = {x, y - height, width, height};
        SDL_RenderCopy(renderer, texture, NULL, &destRect);
    }
}

void Player::update(std::vector<Platform>& platforms) {

    if (isJumping) {
        velocityY += gravity;
        float remainingMovement = velocityY;
        int steps = std::max(1, int(std::abs(remainingMovement)));
        float dy = remainingMovement / steps;

        for (int i = 0; i < steps; ++i) {
            y += dy;
            for (auto& platform : platforms) {
                if (checkPlatformCollision(platform)) {
                    break;
                }
            }
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

    if (x > SCREEN_WIDTH) {
        x = -width;
    }
}

void Player::moveLeft() {
    x -= stepX;
    facingLeft = true;

    if (x < -width) {
        x = SCREEN_WIDTH;
    }
}

bool Player::checkPlatformCollision(Platform& platform) {
    if (velocityY < 0) return false;

    if (platform.isBroken()) return false;

    const int footHeight = 5;
    SDL_Rect footRect = {
        x,
        y - footHeight,
        width,
        footHeight
    };

    SDL_Rect platformRect = platform.getRect();

    if (SDL_HasIntersection(&footRect, &platformRect)) {
        y = platformRect.y;
        velocityY = 0;
        isJumping = false;

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
    texture = facingLeft ? leftTexture : rightTexture;
}

void Player::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

