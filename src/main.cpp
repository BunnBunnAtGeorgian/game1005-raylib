#include "raylib.h"
#include "Math.h"
#include <cassert>
#include <array>
#include <vector>
#include <algorithm>
using namespace std;

constexpr int SCREEN_SIZE = 800;
constexpr int TILE_SIZE = 40;
constexpr int TILE_COUNT = SCREEN_SIZE / TILE_SIZE;
constexpr int BUTTON_POSX = 20;
constexpr int BUTTON_POSY = 20;
constexpr int BUTTON_WIDTH = 200;
constexpr int BUTTON_HEIGHT = 80;

constexpr float BULLET_RADIUS = 15.0f;
constexpr float ENEMY_RADIUS = 20.0f;
constexpr int ENEMY_TOTAL = 10;

constexpr float SPAWN_INTERVALS = 1.0f;

enum TileType : int
{
    GRASS,
    DIRT,
    WAYPOINT,
    TOWER,
    SLOT,
    COUNT
};

struct Cell
{
    int row;
    int col;
};

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

struct Enemy
{
    Vector2 position;
    Vector2 direction;
    int health;
    int damage;
    int currentWaypoint;
    bool enabled = true;
};
struct Bullet
{
    Vector2 position{};
    Vector2 direction{};
    float time = 0.0f;
    bool enabled = true;
};
struct Tower
{
    Cell cell;
    int damage = 5;
    float range = 200.0f;
    float dps = 1.0f;
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
    Color colours[COUNT]{ LIME, BEIGE, SKYBLUE,ORANGE, Color{ 20, 120,60, 255 }  };
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

void UpdateEnemies(vector<Enemy>& enemies, const vector<Cell>& waypoints, float dt) {
    for (Enemy& enemy : enemies) {
        int nextWaypoint = enemy.currentWaypoint + 1;

        Vector2 A = TileCenter (waypoints[enemy.currentWaypoint]); // Current waypoint
        Vector2 B = TileCenter(waypoints[nextWaypoint]); // Next waypoint

        // Move enemy towards the next waypoint
        enemy.direction = Normalize(B - A);
        enemy.position = enemy.position + enemy.direction * 200.0f * dt;

        // If enemy reached the next waypoint
        if (CheckCollisionPointCircle(B, enemy.position, 10.0f)) {
            enemy.position = B;  // Snap
            enemy.currentWaypoint = nextWaypoint;         // Move to next waypoint
            nextWaypoint = (enemy.currentWaypoint + 1) % waypoints.size();
        }
        // if enemy reaches end
            enemies.erase(
                remove_if(enemies.begin(), enemies.end(), [&](Enemy enemy)
                    {
                        if (enemy.currentWaypoint == waypoints.size() - 1) {

                            // Return true if you WANT the element (enemy) to be erased!
                            return enemy.enabled;
                        }
                        return !enemy.enabled;
                    }),
                enemies.end());
        
    }
}

void DrawEnemies(vector<Enemy>& enemies) {
    for (Enemy& enemy : enemies) {
        DrawCircleV(enemy.position, ENEMY_RADIUS, RED);
    }
}

//TowerStuff
//get selected cell by mouse position divided by the size of cell and then ceiling it to get int of row and col
Cell SelectCell() {
    int cellxCol = ceil(GetMousePosition().y / TILE_SIZE) - 1;
    int cellyRow = ceil(GetMousePosition().x / TILE_SIZE) - 1;
    Cell selectedCell = { cellxCol,cellyRow };    
    return selectedCell;
}
void ChangeTile(Cell tilePos, TileType newTileType, int tiles[TILE_COUNT][TILE_COUNT]) {
    tiles[tilePos.row][tilePos.col] = newTileType;
}
bool IsTowerSlot(Cell tilePos, int tiles[TILE_COUNT][TILE_COUNT]) {
    return tiles[tilePos.row][tilePos.col] == SLOT;
}
void DrawTowers(vector<Tower>& towers) {
    for (Tower& tower : towers) {
        Vector2 towerPos = TileCenter(tower.cell);
        DrawCircleV(towerPos, 15.0f, DARKBLUE);
    }
}
const Enemy* InRange(const Tower tower, const vector<Enemy>& enemies) {
    const Enemy* nearestEnemy = nullptr;
    float inRangeDistance = tower.range;
    for (const Enemy& enemy : enemies) {
        float distance = Distance(TileCenter(tower.cell), enemy.position);
        if (distance < inRangeDistance) {
            nearestEnemy = &enemy;
            inRangeDistance = distance;
        }
    }
    return nearestEnemy;
}
float shootCurrent = 0;
void ShootBullets( const vector<Tower>& towers, float dt, vector<Bullet>& bullets, vector<Enemy>& enemies) {
    for (const Tower& tower : towers) {
        // Shoot a bullet every 0.25 seconds if we're holding space
        shootCurrent += dt;
        if (shootCurrent > tower.dps)
        {
            if (InRange(tower, enemies) != nullptr) {
                Vector2 nearestEnemyPos = InRange(tower, enemies)->position;
                Bullet bullet;
                bullet.position = TileCenter(tower.cell);
                bullet.direction = Normalize(nearestEnemyPos - bullet.position);
                bullets.push_back(bullet);
                shootCurrent = 0.0f;
            }

        }
    }
      
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
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 4, 0, 0, 0, 0, 0 }, // 0
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 0, 0, 0 }, // 1
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0 }, // 2
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 0, 0, 0 }, // 3
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0 }, // 4
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 0, 0, 0 }, // 5
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4, 0, 0, 0, 0, 0 }, // 6
        { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 4, 0, 0, 0, 0 }, // 7
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0 }, // 8
        { 0, 0, 0, 1, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0 }, // 9
        { 0, 0, 0, 1, 0, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0, 0 }, // 10
        { 0, 0, 0, 1, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0 }, // 11
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0 }, // 12
        { 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 4 }, // 13
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4, 0 }, // 14
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4 }, // 15
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4, 0 }, // 16
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 4 }, // 17
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0 }, // 18
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 4, 0, 4, 0, 4, 0, 0, 0 }  // 19
    };
    
    // Automatic approach:
    vector<Cell> waypoints = FloodFill({ 0, 12 }, tiles, WAYPOINT);
    vector<Tower> towers;
    int curr = 0;
    int next = curr + 1;
    Vector2 startPosition = TileCenter(waypoints[curr]);
    float enemySpeed = 250.0f;
    bool atEnd = false;

    float spawnTimer = 0.0f;

    vector<Bullet> bullets;
    float bulletSpeed = 500.0f;
    float shootCurrent = 0.0f;
    float shootTotal = 0.25f;

    vector<Enemy>enemies;
    
    InitWindow(800, 800, "Bloons TD6");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        //game state switch
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (IsTowerSlot(SelectCell(), tiles))
            {
                ChangeTile(SelectCell(), TOWER, tiles);
                Tower newTower = { SelectCell() };
                towers.push_back(newTower);
            }
                //  ++game.state %= 3;
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



    //  if (CheckCollisionPointCircle(B, enemyPosition, 10.0f))
    //  {
    //      enemyPosition = B;
    //      ++curr %= waypoints.size();
    //      ++next %= waypoints.size();
    //      // TODO -- Fix this (add an actual condition to check if the enemy has reached the end)
    //  }
        

        // Bullet update
      for (int i = 0; i < bullets.size(); i++)
      {
          Bullet& bullet = bullets[i];
          bullet.position = bullet.position + bullet.direction * bulletSpeed * dt;
          bullet.time += dt;
          for (Enemy& enemy : enemies) {
              bool collision = CheckCollisionCircles(bullet.position, BULLET_RADIUS, enemy.position, ENEMY_RADIUS);
              bool expired = bullet.time >= 0.5f;
              bullet.enabled = !collision && !expired;
          }
      }
      
       bullets.erase(
           std::remove_if(bullets.begin(), bullets.end(), [&](Bullet& bullet)
           {
                   // Return true if you WANT the element (bullet) to be erased!
                   return !bullet.enabled;
           }), 
       bullets.end());

        spawnTimer += dt;

        if (enemies.size() < ENEMY_TOTAL && spawnTimer >= SPAWN_INTERVALS) {
            printf("enemy spawned");
            Enemy newEnemy = { startPosition, {0,0},100,100 };
            enemies.push_back(newEnemy);
            spawnTimer = 0.0f;
        }
        
        ShootBullets(towers, dt, bullets, enemies);
        
        UpdateEnemies(enemies, waypoints, dt);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                DrawTile(row, col, tiles[row][col]);
            }
        }
        //draw enemies
        DrawEnemies(enemies);
        DrawTowers(towers);

        //draw bullets
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
