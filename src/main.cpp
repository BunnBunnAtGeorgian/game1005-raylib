#include "raylib.h"
#include "Math.h"
#include <cassert>
#include <array>
#include <vector>
#include <algorithm>

constexpr int SCREEN_SIZE = 800;
constexpr int TILE_SIZE = 40;
constexpr int TILE_COUNT = SCREEN_SIZE / TILE_SIZE;
constexpr int BUTTON_POSX = 20;
constexpr int BUTTON_POSY = 20;
constexpr int BUTTON_WIDTH = 200;
constexpr int BUTTON_HEIGHT = 80;

constexpr float BULLET_RADIUS = 15.0f;
constexpr float ENEMY_RADIUS = 25.0f;

enum TileType : int
{
    GRASS,
    DIRT,
    WAYPOINT,
    TOWER,
    COUNT
};

struct Cell
{
    int row;
    int col;
};

Vector2 TileCenter(Cell cell)
{
    Vector2 pixel{ cell.col * TILE_SIZE, cell.row * TILE_SIZE };    // Top-left
    pixel = pixel + TILE_SIZE * 0.5f;   // Center
    return pixel;
}

constexpr std::array<Cell, 4> DIRECTIONS{ Cell{ -1, 0 }, Cell{ 1, 0 }, Cell{ 0, -1 }, Cell{ 0, 1 } };

inline bool InBounds(Cell cell, int rows = TILE_COUNT, int cols = TILE_COUNT)
{
    return cell.col >= 0 && cell.col < cols && cell.row >= 0 && cell.row < rows;
}

void DrawTile(int row, int col, Color color)
{
    DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, color);
}

void DrawTile(int row, int col, int tileType)
{
    assert(tileType >= 0 && tileType < COUNT);
    Color colours[COUNT]{ LIME, BEIGE, SKYBLUE, Color{ 20, 120,60, 255 } };
    DrawTile(row, col, colours[tileType]);
}

// Returns a collection of adjacent cells that match the search value.
std::vector<Cell> FloodFill(Cell start, int tiles[TILE_COUNT][TILE_COUNT], TileType searchValue)
{
    // "open" = "places we want to search", "closed" = "places we've already searched".
    std::vector<Cell> result;
    std::vector<Cell> open;
    bool closed[TILE_COUNT][TILE_COUNT];
    for (int row = 0; row < TILE_COUNT; row++)
    {
        for (int col = 0; col < TILE_COUNT; col++)
        {
            // We don't want to search zero-tiles, so add them to closed!
            closed[row][col] = tiles[row][col] == 0;
        }
    }

    // Add the starting cell to the exploration queue & search till there's nothing left!
    open.push_back(start);
    while (!open.empty())
    {
        // Remove from queue and prevent revisiting
        Cell cell = open.back();
        open.pop_back();
        closed[cell.row][cell.col] = true;

        // Add to result if explored cell has the desired value
        if (tiles[cell.row][cell.col] == searchValue)
            result.push_back(cell);

        // Search neighbours
        for (Cell dir : DIRECTIONS)
        {
            Cell adj = { cell.row + dir.row, cell.col + dir.col };
            if (InBounds(adj) &&
                !closed[adj.row][adj.col] &&
                tiles[adj.row][adj.col] > 0)
                open.push_back(adj);
        }
    }

    return result;
}
//gray button with next
//green during play
//turn to gray for next game
//if lose turn blue for new game
enum State : int {
    BEGIN,
    PLAY,
    END
};
struct Game
{
    Rectangle button;
    Color buttonColour;
    int state;
};
//GAMESTATES
void UpdateBegin(Game& game) {
    // Add mouse-out vs mouse-over colour
    Color buttonColorOut = ORANGE;
    Color buttonColorIn = Color{ 205, 111, 0, 255 };
    bool mouseOver = CheckCollisionPointRec(GetMousePosition(), game.button);
    game.buttonColour = mouseOver ? buttonColorIn : buttonColorOut;
    if (mouseOver && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        ++game.state %= 3;
}
void UpdatePlay(Game& game)
{
    // Replace this with actual win condition to transition from pong play to pong game over
    if (CheckCollisionPointRec(GetMousePosition(), game.button) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        ++game.state %= 3;
}

void UpdateEnd(Game& game)
{
    // Replace with a play again vs quit option?
    if (CheckCollisionPointRec(GetMousePosition(), game.button) && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        ++game.state %= 3;
}

void DrawBegin(Game& game)
{
    DrawRectangleRec(game.button, game.buttonColour);
}

void DrawPlay(Game& game)
{
    DrawRectangleRec(game.button, Color{ 70, 128, 158, 255 });
}

void DrawEnd(Game& game)
{
    DrawRectangleRec(game.button, Color{ 50, 50, 50, 255 });
}

struct Bullet
{
    Vector2 position{};
    Vector2 direction{};
    float time = 0.0f;
    bool enabled = true;
};

struct Enemy
{
    Vector2 Position;
    int health;
    int speed;
    int damage;
};
struct Tower
{
    int damage;
    float range;
    float dps;
};
Tower towers[];
Enemy enemies[];

void SpawnEnemy() {
    
}
int main()
{
    Game game;
    game.state = BEGIN;
    game.button.x = BUTTON_POSX;
    game.button.y = BUTTON_POSY;
    game.button.width = BUTTON_WIDTH;
    game.button.height = BUTTON_HEIGHT;
    
    int tiles[TILE_COUNT][TILE_COUNT]
    {
    //col:0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19    row:
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0 }, // 0
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 1
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 2
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 3
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 4
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 5
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 }, // 6
        { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0 }, // 7
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 8
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 9
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 10
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 11
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 12
        { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 13
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 14
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 15
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }, // 16
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0 }, // 17
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 18
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }  // 19
    };
    
    // Automatic approach:
    std::vector<Cell> waypoints = FloodFill({ 0, 12 }, tiles, WAYPOINT);
    int curr = 0;
    int next = curr + 1;
    Vector2 enemyPosition = TileCenter(waypoints[curr]);
    float enemySpeed = 250.0f;
    bool atEnd = false;

    std::vector<Bullet> bullets;
    float bulletSpeed = 500.0f;
    float shootCurrent = 0.0f;
    float shootTotal = 0.25f;

    InitWindow(800, 800, "Game");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        //game state switch
        if (IsKeyPressed(KEY_SPACE))
        {
            ++game.state %= 3;
        }

        switch (game.state)
        {
        case BEGIN:
            UpdateBegin(game);
            break;

        case PLAY:
            UpdatePlay(game);
            break;

        case END:
            UpdateEnd(game);
            break;
        }

        float dt = GetFrameTime();
        Vector2 mouse = GetMousePosition();
        if (IsKeyDown(KEY_SPACE))
        {
            // Shoot a bullet every 0.25 seconds if we're holding space
            shootCurrent += dt;
            if (shootCurrent > shootTotal)
            {
                shootCurrent = 0.0f;

                Bullet bullet;
                bullet.position = mouse;
                bullet.direction = Normalize(enemyPosition - bullet.position);
                bullets.push_back(bullet);
            }
        }

        Vector2 A = TileCenter(waypoints[curr]);
        Vector2 B = TileCenter(waypoints[next]);
        Vector2 enemyDirection = Normalize(B - A);
        enemyPosition = enemyPosition + enemyDirection * enemySpeed * dt;
        if (CheckCollisionPointCircle(B, enemyPosition, 10.0f))
        {
            enemyPosition = B;
            ++curr %= waypoints.size();
            ++next %= waypoints.size();
            // TODO -- Fix this (add an actual condition to check if the enemy has reached the end)
        }

        // Bullet update
        for (int i = 0; i < bullets.size(); i++)
        {
            Bullet& bullet = bullets[i];
            bullet.position = bullet.position + bullet.direction * bulletSpeed * dt;
            bullet.time += dt;

            bool collision = CheckCollisionCircles(bullet.position, BULLET_RADIUS, enemyPosition, ENEMY_RADIUS);
            bool expired = bullet.time >= 1.0f;
            bullet.enabled = !collision && !expired;
        }

        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(), [&](Bullet& bullet)
            {
                    // Return true if you WANT the element (bullet) to be erased!
                    return !bullet.enabled;
            }), 
        bullets.end());

        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                DrawTile(row, col, tiles[row][col]);
            }
        }
        DrawCircleV(enemyPosition, 25.0f, PURPLE);
        for (int i = 0; i < bullets.size(); i++)
        {
            DrawCircleV(bullets[i].position, 15.0f, RED);
        }

        switch (game.state)
        {
        case BEGIN:
            DrawBegin(game);
            break;

        case PLAY:
            DrawPlay(game);
            break;

        case END:
            DrawEnd(game);
            break;
        }

        EndDrawing();
    }
    // yuh
    CloseWindow();
    return 0;
}
