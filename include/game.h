#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "game_level.h"

enum GameState
{
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};
// Initial size of the player paddle

// Initial velocity of the player paddle

class Game
{
public:
    glm::vec2 PLAYER_SIZE = glm::vec2(20.0f, 20.0f);
    float PLAYER_VELOCITY = 200.0f;
    float ENEMY_VELOCITY = 50.0f;
    GameState State;
    int light = 0;
    bool Keys[1024];
    unsigned int Width, Height;
    std::vector<GameLevel> Levels;
    unsigned int Level;
    Game(unsigned int width, unsigned int height);
    ~Game();

    void Init();
    void ProcessInput(float dt);
    void Update(float dt);
    
    void DoCollisions();
    // bool CheckCollision(GameObject &one, GameObject &two); // AABB - AABB collision
    void Render();
};

#endif