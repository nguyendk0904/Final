#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED
#include <SDL.h>
#include <vector>
#include <random>

enum class PlatformType {
    NORMAL,
    MOVING,
    BREAKABLE
};

class Platform {
private:
    SDL_Rect rect;
    PlatformType type;
    float speed;
    int direction;
    int screenWidth;
    SDL_Texture* texture;
    bool broken;
    int breakTimer;

public:
    Platform(int x, int y, int width, int height, PlatformType platformType = PlatformType::NORMAL);
    ~Platform();

    void render(SDL_Renderer* renderer);
    void update();
    void startBreaking();

    SDL_Rect getRect() const { return rect; }
    PlatformType getType() const { return type; }
    bool isMoving() const { return type == PlatformType::MOVING; }
    bool isBreakable() const { return type == PlatformType::BREAKABLE; }
    bool isBroken() const { return broken; }

    void setScreenWidth(int width) { screenWidth = width; }
    void setTexture(SDL_Texture* newTexture) { texture = newTexture; }
    void setY(int newY) { rect.y = newY; }
};

class PlatformManager {
private:
    std::vector<Platform> platforms;
    int screenWidth;
    int screenHeight;
    int platformWidth;
    int platformHeight;
    std::mt19937 rng;
    std::uniform_int_distribution<int> xDist;
    std::uniform_int_distribution<int> typeDist;
    SDL_Texture* platformTexture;
    SDL_Texture* movingPlatformTexture;
    SDL_Texture* breakablePlatformTexture;
    int difficultyLevel;
    int platformsPerLevel;
    int basePlatformCount;

public:
    PlatformManager(int screenWidth, int screenHeight);
    ~PlatformManager();

    void initialize(int numPlatforms);
    void render(SDL_Renderer* renderer);
    void update();

    void scrollPlatforms(float scrollAmount);
    void removeBottomPlatforms();
    void addNewPlatforms(int numToAdd);

    void setTextures(SDL_Texture* normalTexture, SDL_Texture* movingTexture = nullptr, SDL_Texture* breakableTexture = nullptr);

    const std::vector<Platform>& getPlatforms() const { return platforms; }
    std::vector<Platform>& getPlatforms() { return platforms; }
    bool isOverlapping(int x, int y) const;

    void updateDifficulty(int score);
    int getPlatformsToGenerate() const;
};

#endif // PLATFORM_H_INCLUDED
