#include <bits/stdc++.h>
#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "game_level.h"
#include "text_renderer.h"

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <string.h>
#include <chrono>
using namespace std::chrono;
using namespace std;
std::chrono::_V2::system_clock::time_point start;

std::chrono::_V2::system_clock::time_point last;

SpriteRenderer *Renderer;
TextRenderer *Text;
GameObject *Player;
vector<GameObject *> enemy[3];

glm::vec2 player_pos;

int left_boundary = 00.0f;
int right_boundary = 800.0f;
int top_boundary = 00.0f;
int bottom_boundary = 600.0f;
int coins = 0;
bool won = false;

std::chrono::_V2::system_clock::time_point enemy_start;

std::chrono::_V2::system_clock::time_point enemy_last;

std::chrono::_V2::system_clock::time_point from_start;

std::chrono::_V2::system_clock::time_point to_start;

float score = 0.0f;

float in_light = 0.1f;
float out_light = 0.01f;

time_t enemy_move = 0.5;

vector<int> enemy_dir[3];

int levelWidth = 800;
int levelHeight = 600;
unsigned int height = 50;
unsigned int width = 60; // note we can index vector at [0] since this function is only called if height > 0
float unit_width = (levelWidth) / static_cast<float>(width), unit_height = (levelHeight) / height;

const int num_x = 50;
const int num_y = 60;
int player_x = 0;
int player_y = 0;

// number of blocks

int level_[] = {7, 10, 13};

int level[3][num_x][num_y];

vector<bool> level_bricks[3];
vector<bool> level_coins[3];
vector<bool> level_pipes[3];

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}
int dx[] = {0, 0, 1, -1};
int dy[] = {1, -1, 0, 0};

bool check(int lev, int x, int y)
{
    for (int i = 0; i < 4; i++)
    {
        int xx = x + dx[i];
        int yy = y + dy[i];
        if (xx >= 0 && xx < num_x && yy >= 0 && yy < num_y)
        {
            if (level[lev][xx][yy] != 0)
                return false;
        }
    }
    return true;
}
bool check_dist(int x, int y, int lev)
{
    int d = abs(x - 25) + abs(y - 30);
    if (d < 3)
    {
        return false;
    }
    return true;
}

Game::~Game()
{
    delete Renderer;
    delete Player;
}

vector<string> construct(int lev, int xx, int yy, int zz)
{
    //
    for (int i = 0; i < num_x; i++)
    {
        for (int j = 0; j < num_y; j++)
        {
            if (i == 0 || i == num_x - 1 || j == 0 || j == num_y - 1)
            {
                level[lev][i][j] = 1;
            }
            else
            {
                level[lev][i][j] = 0;
            }
        }
    }
    enemy_dir[lev] = vector<int>(zz);
    for (int i = 0; i < zz; i++)
    {
        enemy_dir[lev][i] = rand() % 4;
    }

    int taken_bricks = 0;
    int taken_coins = 0;
    int taken_enemy = 0;
    while (taken_bricks < xx)
    {
        int x = rand() % num_x;
        int y = rand() % num_y;
        if (x == 25 && y == 30)
            continue;
        if (level[lev][x][y] == 0 && check(lev, x, y) && check_dist(x, y, lev))
        {
            level[lev][x][y] = 2;
            taken_bricks++;
        }
    }
    while (taken_coins < yy)
    {
        int x = rand() % num_x;
        int y = rand() % num_y;
        if (x == 25 && y == 30)
            continue;
        if (level[lev][x][y] == 0 && check(lev, x, y) && check_dist(x, y, lev))
        {
            level[lev][x][y] = 3;
            taken_coins++;
        }
    }
    enemy[lev] = vector<GameObject *>(zz);
    while (taken_enemy < zz)
    {
        int x = rand() % num_x;
        int y = rand() % num_y;
        if (x == 25 && y == 30)
            continue;
        if (level[lev][x][y] == 0 && check(lev, x, y) && check_dist(x, y, lev))
        {
            // level[lev][x][y] = 6;
            glm::vec2 enemyPos = glm::vec2(y * unit_width, x * unit_height);
            //             // cout << "1" << endl;
            glm::vec2 ENEMY_SIZE = glm::vec2(unit_width, unit_height);
            enemy[lev][taken_enemy] = new GameObject(enemyPos, ENEMY_SIZE, ResourceManager::GetTexture("enemy"));
            //             // cout << enemyPos.x << " " << enemyPos.y << endl;

            taken_enemy++;
        }
    }
    player_x = 30 * unit_width;
    player_y = 25 * unit_height;

    level[lev][(num_x / 2) - 1][(num_y)-1] = 4;
    level[lev][num_x / 2][num_y - 1] = 4;
    if (lev == 2)
    {
        level[lev][(num_x / 2) - 1][(num_y)-1] = 5;
        level[lev][num_x / 2][num_y - 1] = 5;
    }

    std::vector<string> level_str(num_x);
    for (int i = 0; i < num_x; i++)
    {
        for (int j = 0; j < num_y; j++)
        {
            level_str[i] += std::to_string(level[lev][i][j]);
            level_str[i] += " ";
        }
    }
    int height = num_x;
    int width = num_y;
    int c = 0;
    level_coins[lev] = vector<bool>(height * width);
    level_bricks[lev] = vector<bool>(height * width);
    level_pipes[lev] = vector<bool>(height * width);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (level[lev][i][j] == 0)
            {
                continue;
            }

            if (level[lev][i][j] == 3)
            {
                level_coins[lev][c] = true;
            }
            else
            {
                level_coins[lev][c] = false;
            }
            if (level[lev][i][j] == 2)
            {
                level_bricks[lev][c] = true;
            }
            else
            {
                level_bricks[lev][c] = false;
            }
            if (level[lev][i][j] == 4 || level[lev][i][j] == 5)
            {
                level_pipes[lev][c] = true;
            }
            else
            {
                level_pipes[lev][c] = false;
            }
            c++;
        }
    }
    return level_str;
}

void Game::Init()
{
    srand(time(0));
    from_start = high_resolution_clock::now();
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("/home/aryan/sem-4/cg/Assignment_1/fonts/OCRAEXT.TTF", 24);
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // load textures
    ResourceManager::LoadTexture("textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("textures/enemy.png", false, "enemy");
    ResourceManager::LoadTexture("textures/fgate.jpg", false, "fgate");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/player.bmp", false, "player");
    ResourceManager::LoadTexture("textures/coin.jpeg", false, "coins");
    ResourceManager::LoadTexture("textures/door.jpg", false, "door");
    ResourceManager::LoadTexture("textures/gameOver.jpg", false, "gameOver");
    ResourceManager::LoadTexture("textures/won.jpeg", false, "gameWon");

    ofstream myfile1("/home/aryan/sem-4/cg/Assignment_1/levels/one.lvl");
    ofstream myfile2("/home/aryan/sem-4/cg/Assignment_1/levels/two.lvl");
    ofstream myfile3("/home/aryan/sem-4/cg/Assignment_1/levels/three.lvl");

    vector<string> level_str_0 = construct(0, level_[0], level_[0], level_[0]);
    vector<string> level_str_1 = construct(1, level_[1], level_[1], level_[1]);
    vector<string> level_str_2 = construct(2, level_[2], level_[2], level_[2]);

    for (int i = 0; i < num_x; i++)
    {
        myfile1 << level_str_0[i] << endl;
    }
    myfile1.close();

    for (int i = 0; i < num_x; i++)
    {
        myfile2 << level_str_1[i] << endl;
    }
    myfile2.close();

    for (int i = 0; i < num_x; i++)
    {
        myfile3 << level_str_2[i] << endl;
    }
    myfile3.close();
    // load levels
    GameLevel one;
    one.Load("levels/one.lvl", this->Width, this->Height);
    GameLevel two;
    two.Load("levels/two.lvl", this->Width, this->Height);
    GameLevel three;
    three.Load("levels/three.lvl", this->Width, this->Height);

    ////
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Level = 0;

    glm::vec2 playerPos = glm::vec2(player_x, player_y);
    glm::vec2 PLAYER_SIZE = glm::vec2(unit_width, unit_height);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("player"));
    enemy_start = high_resolution_clock::now();
}
bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    //   cout << "jhassd" << endl;
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
                      two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
                      two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}
void Game::Update(float dt)
{
    if (this->State == GAME_WIN)
    {
        return;
    }
    if (this->light == 1)
    {
        score += in_light;
    }
    else
    {
        score += out_light;
    }

    player_pos = Player->Position;
    float velocity = ENEMY_VELOCITY * dt;
    int c = 0;
    int d = 0;
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (level_pipes[this->Level][c])
        {
            d++;
            if (d == 2 && CheckCollision(*Player, box))
            {
                this->Level++;
                if (this->Level == 1)
                {
                    PLAYER_VELOCITY = 150.0f;
                    ENEMY_VELOCITY = 100.0f;
                }
                else if (this->Level == 2)
                {
                    PLAYER_VELOCITY = 100.0f;
                    ENEMY_VELOCITY = 150.0f;
                }
                else if (this->Level == 3)
                {
                    won = true;
                    this->State = GameState::GAME_WIN;
                    Renderer->DrawSprite(ResourceManager::GetTexture("gameWon"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f, glm::vec3(1.0f), player_pos, light);
                    start = high_resolution_clock::now();
                    break;
                }

                Player->Position = glm::vec2(player_x, player_y);
            }
        }
        c++;
    }

    if (this->State == GAME_WIN)
    {
        return;
    }

    // collsion with enemy
    for (int i = 0; i < enemy[this->Level].size(); i++)
    {
        if (CheckCollision(*Player, *enemy[this->Level][i]))
        {
            this->State = GameState::GAME_WIN;
            Renderer->DrawSprite(ResourceManager::GetTexture("gameOver"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f, glm::vec3(1.0f), player_pos, 0);
            start = high_resolution_clock::now();
            break;
        }
    }
    if (this->State == GAME_WIN)
    {
        return;
    }

    // movement of enemy

    for (int i = 0; i < enemy[this->Level].size(); i++)
    {
        int r = enemy_dir[this->Level][i];
        // int r = rand() % 4;
        if (r == 0)
        {
            if (enemy[this->Level][i]->Position.x >= left_boundary + unit_width)
                enemy[this->Level][i]->Position.x -= velocity;
            int c = 0;
            bool is = false;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_bricks[this->Level][c])
                {

                    if (CheckCollision(*enemy[this->Level][i], box))
                    {
                        is = true;
                        break;
                    }
                }
                c++;
            }
            if (is)
            {
                enemy[this->Level][i]->Position.x += velocity;
            }
        }
        else if (r == 1)
        {
            if (enemy[this->Level][i]->Position.x <= right_boundary - enemy[this->Level][i]->Size.x - unit_width)
                enemy[this->Level][i]->Position.x += velocity;
            int c = 0;
            bool is = false;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_bricks[this->Level][c])
                {

                    if (CheckCollision(*enemy[this->Level][i], box))
                    {
                        is = true;
                        break;
                    }
                }
                c++;
            }
            if (is)
            {
                enemy[this->Level][i]->Position.x -= velocity;
            }
        }
        else if (r == 2)
        {
            if (enemy[this->Level][i]->Position.y >= top_boundary + unit_height)
                enemy[this->Level][i]->Position.y -= velocity;
            int c = 0;
            bool is = false;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_bricks[this->Level][c])
                {

                    if (CheckCollision(*enemy[this->Level][i], box))
                    {
                        is = true;
                        break;
                    }
                }
                c++;
            }
            if (is)
            {
                enemy[this->Level][i]->Position.y += velocity;
            }
        }
        else if (r == 3)
        {
            if (enemy[this->Level][i]->Position.y <= bottom_boundary - enemy[this->Level][i]->Size.y - unit_height)
                enemy[this->Level][i]->Position.y += velocity;
            int c = 0;
            bool is = false;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_bricks[this->Level][c])
                {

                    if (CheckCollision(*enemy[this->Level][i], box))
                    {
                        is = true;
                        break;
                    }
                }
                c++;
            }
            if (is)
            {
                enemy[this->Level][i]->Position.y -= velocity;
            }
        }
    }
    enemy_last = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(enemy_last - enemy_start);
    if (duration.count() > 0.1)
    {
        enemy_start = high_resolution_clock::now();
        for (int i = 0; i < enemy_dir[this->Level].size(); i++)
        {

            int rr = rand() % 4;
            while (rr == enemy_dir[this->Level][i])
            {
                rr = rand() % 4;
            }
            enemy_dir[this->Level][i] = rr;
        }
    }
}
void Game::DoCollisions()
{
}

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;
        if (this->Keys[GLFW_KEY_A])
        {
            if (Player->Position.x >= left_boundary + unit_width)
                Player->Position.x -= velocity;
            int c = 0;
            bool is = false;
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_bricks[this->Level][c])
                {

                    if (CheckCollision(*Player, box))
                    {
                        is = true;
                        break;
                    }
                }
                c++;
            }
            c = 0;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_coins[this->Level][c])
                {

                    if (CheckCollision(*Player, box) && box.Destroyed == false)
                    {
                        coins++;

                        box.Destroyed = true;
                    }
                }
                c++;
            }
            if (is)
            {
                Player->Position.x += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Player->Position.x <= right_boundary - Player->Size.x - unit_width)
                Player->Position.x += velocity;
            int c = 0;
            bool is = false;
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_bricks[this->Level][c])
                {

                    if (CheckCollision(*Player, box))
                    {

                        is = true;
                        break;
                    }
                }
                c++;
            }
            c = 0;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_coins[this->Level][c])
                {

                    if (CheckCollision(*Player, box) && box.Destroyed == false)
                    {
                        coins++;
                        box.Destroyed = true;
                    }
                }
                c++;
            }
            if (is)
            {
                Player->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_W])
        {
            if (Player->Position.y >= top_boundary + unit_height)
                Player->Position.y -= velocity;
            int c = 0;
            bool is = false;
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_bricks[this->Level][c])
                {

                    if (CheckCollision(*Player, box))
                    {

                        is = true;
                        break;
                    }
                }
                c++;
            }
            c = 0;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_coins[this->Level][c])
                {

                    if (CheckCollision(*Player, box) && box.Destroyed == false)
                    {
                        coins++;
                        box.Destroyed = true;
                    }
                }
                c++;
            }
            if (is)
            {
                Player->Position.y += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_S])
        {
            if (Player->Position.y <= bottom_boundary - Player->Size.y - unit_height)
                Player->Position.y += velocity;
            int c = 0;
            bool is = false;
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_bricks[this->Level][c])
                {

                    if (CheckCollision(*Player, box))
                    {

                        is = true;
                        break;
                    }
                }
                c++;
            }
            c = 0;
            for (GameObject &box : this->Levels[this->Level].Bricks)
            {
                if (level_coins[this->Level][c])
                {
                    if (CheckCollision(*Player, box) && box.Destroyed == false)
                    {
                        coins++;
                        box.Destroyed = true;
                    }
                }
                c++;
            }
            if (is)
            {
                Player->Position.y -= velocity;
            }
        }
    }
}
int tt = 0;

void Game::Render()
{
    to_start = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(to_start - from_start);
    if (this->State == GAME_ACTIVE)
        tt = floor(duration.count());
    int s = floor(score);
    std::stringstream ss;
    ss << s;
    std::stringstream ff;
    ff << coins;
    stringstream gg;
    gg << tt;

    if (this->State == GAME_ACTIVE)
    {

        // draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f, glm::vec3(1.0f), player_pos, light);
        // draw level
        this->Levels[this->Level].Draw(*Renderer, player_pos, light);
        // draw player
        Player->Draw(*Renderer, player_pos, light);
        for (int i = 0; i < enemy[this->Level].size(); i++)
        {
            enemy[this->Level][i]->Draw(*Renderer, player_pos, light);
        }
        std::stringstream ss;
        ss << s;
        std::stringstream ff;
        ff << coins;
        Text->RenderText("Score:" + ss.str() + "  " + "Coins:" + ff.str() + " Time:" + gg.str(), 255.0f, 305.0f, 1.0f);
    }
    else if (this->State == GAME_WIN)
    {
        if (!won)
        {
            Renderer->DrawSprite(ResourceManager::GetTexture("gameOver"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f, glm::vec3(1.0f), player_pos, light);
        }
        else
        {
            Renderer->DrawSprite(ResourceManager::GetTexture("gameWon"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f, glm::vec3(1.0f), player_pos, light);
        }
        last = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(last - start);
        Text->RenderText("Score:" + ss.str() + "  " + "Coins:" + ff.str() + " Time:" + gg.str(), 255.0f, 305.0f, 1.0f);
        if (duration.count() > 2)
        {
            exit(0);
        }
    }
}
