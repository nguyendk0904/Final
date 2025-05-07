#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <SDL.h>
#include <SDL_mixer.h>
#include <vector>
#include "platform.h"

class Player {
private:
    int x, y;
    int width, height;
    float stepX;
    bool isJumping;
    float velocityY;
    float gravity;
    float jumpStrength;
    SDL_Texture* texture;
    bool facingLeft;
    Mix_Chunk* jumpSound;
    bool isFallingSoundPlaying;
    float previousVelocityY;

public:
    Player(int startX, int startY, int size);
    ~Player();

    void render(SDL_Renderer* renderer);
    void update(std::vector<Platform>& platforms);
    void jump();
    void moveRight();
    void moveLeft();
    bool checkPlatformCollision(Platform& platform);

    void setTexture(SDL_Texture* texture);
    void setTextures(SDL_Texture* leftTexture, SDL_Texture* rightTexture);

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool getIsJumping() const { return isJumping; }

    void setPosition(int newX, int newY);
    void setJumpSound (Mix_Chunk* sound);


};

#endif // PLAYER_H_INCLUDED
