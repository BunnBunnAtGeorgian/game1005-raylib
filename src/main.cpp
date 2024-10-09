#include "raylib.h"
#include "Math.h"
constexpr float SCREEN_WIDTH = 1200.0f;
constexpr float SCREEN_HEIGHT = 800.0f;
constexpr Vector2 CENTER{ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };

// Ball can move half the screen width per-second
constexpr float BALL_SPEED = SCREEN_WIDTH * 0.5f;
constexpr float BALL_SIZE = 60.0f;

// Paddles can move half the screen height per-second
constexpr float PADDLE_SPEED = SCREEN_HEIGHT * 0.5f;
constexpr float PADDLE_WIDTH = 40.0f;
constexpr float PADDLE_HEIGHT = 80.0f;

float ballSpeedIncrease = 1;

Texture2D Ball;
Texture2D FastBall;
Texture2D BackgroundImg;


struct Box
{
    float xMin;
    float xMax;
    float yMin;
    float yMax;
};

//add circle because balls are round
struct Circle
{
    float xMin;
    float xMax;
    float yMin;
    float yMax;
    float radius;
};
Rectangle BoxToRec(Box box)
{
    Rectangle rec;
    rec.x = box.xMin;
    rec.y = box.yMin;
    rec.width = box.xMax - box.xMin;
    rec.height = box.yMax - box.yMin;
    return rec;
}

Circle BallBox(Vector2 position)
{
    Circle ball;
    ball.xMin = position.x - BALL_SIZE * 0.5f;
    ball.xMax = position.x + BALL_SIZE * 0.5f;
    ball.yMin = position.y - BALL_SIZE * 0.5f;
    ball.yMax = position.y + BALL_SIZE * 0.5f;
    ball.radius = BALL_SIZE * 0.5;
    return ball;
}
Circle BallBox(Vector2 position, float size)
{
    Circle ball;
    ball.xMin = position.x - BALL_SIZE * 0.5f;
    ball.xMax = position.x + BALL_SIZE * 0.5f;
    ball.yMin = position.y - BALL_SIZE * 0.5f;
    ball.yMax = position.y + BALL_SIZE * 0.5f;
    ball.radius = BALL_SIZE * size * 0.5;
    return ball;
}

Box PaddleBox(Vector2 position, float size)
{
    Box box;
    box.xMin = position.x - PADDLE_WIDTH * size * 0.5f;
    box.xMax = position.x + PADDLE_WIDTH * size * 0.5f;
    box.yMin = position.y - PADDLE_HEIGHT * size * 0.5f;
    box.yMax = position.y + PADDLE_HEIGHT * size * 0.5f;
    return box;
};
Box PaddleBox(Vector2 position)
{
    Box box;
    box.xMin = position.x - PADDLE_WIDTH * 0.5f;
    box.xMax = position.x + PADDLE_WIDTH * 0.5f;
    box.yMin = position.y - PADDLE_HEIGHT * 0.5f;
    box.yMax = position.y + PADDLE_HEIGHT * 0.5f;
    return box;
}

bool BoxOverlap(Circle box1, Box box2)
{
    bool x = box1.xMax >= box2.xMin && box1.xMin <= box2.xMax;
    bool y = box1.yMax >= box2.yMin && box1.yMin <= box2.yMax;
    return x && y;
}



void ResetBall(Vector2& position, Vector2& direction)
{
    position = CENTER;
    direction.x = rand() % 2 == 0 ? -1.0f : 1.0f;
    direction.y = 0.0f;
    direction = Rotate(direction, Random(0.0f, 360.0f) * DEG2RAD);
    ballSpeedIncrease = 1;
}

//random direction everytime it hits the wall or paddle so its silly fun and unpredicable pong.
//Makes the game unpredictable.

void ChangeDirection(Vector2& direction) {
    direction = Rotate(direction, Random(direction.x - 45, direction.x + 45) * DEG2RAD);
}

void DrawBall(Vector2 position, Color color, Texture2D ballTexture)
{
    Circle ballBox = BallBox(position);
    Circle ballBoxOutline = BallBox(position,1.3);
    DrawCircle(position.x, position.y, ballBoxOutline.radius, BLACK);
    DrawCircle(position.x, position.y, ballBox.radius, color);
    //DrawRectangle(position.x - BALL_SIZE * 0.5f, position.y - BALL_SIZE * 0.5f, BALL_SIZE, BALL_SIZE, PINK);
    DrawTextureEx(ballTexture, { position.x - BALL_SIZE*0.5f, position.y - BALL_SIZE * 0.5f }, 0, BALL_SIZE / ballTexture.width, WHITE);
}

void DrawPaddle(Vector2 position, Color color)
{
    Box paddleBox = PaddleBox(position,1);
    Box paddleBoxOutline = PaddleBox(position,1.2);

    DrawRectangleRec(BoxToRec(paddleBoxOutline), BLACK);
    DrawRectangleRec(BoxToRec(paddleBox), color);
}


//countdown help from nick

bool isCountdown;
float countdownStartTime;
int counter = 5;
int redPoints = 0;
int bluePoints = 0;
bool isGameOver;
bool isStartCD;
int countDownTime;
void ResetScore() {
    redPoints = 0;
    bluePoints = 0;
}

void DisplayScore(int fontSize, int bluePoints, int redPoints) {
    int centerText = SCREEN_WIDTH / 2;
    DrawText(TextFormat("%d", bluePoints), centerText - 30, 10, fontSize, BLUE);
    DrawText(TextFormat("%d", redPoints), centerText + 30, 10, fontSize, RED);
}
void DisplayText() {
    DrawText(TextFormat("%f", ballSpeedIncrease), 10, 10, 80, RED);
}

void DisplayWinner(int winner, Color color) {
    float centerText = MeasureText("BLUE WINS! OWO", 100) / 2;
    float centerScreenWIDTH = SCREEN_WIDTH / 2;
    float centerScreenHEIGHT = SCREEN_HEIGHT / 2;
    if (isGameOver) {
        //blue won
        if (winner == 1)
            DrawText("BLUE WINS! OWO", centerScreenWIDTH - centerText, centerScreenWIDTH, 100, color);
        //red won
        if (winner == 2)
            DrawText("RED WINS! UWU", centerScreenWIDTH - centerText, centerScreenWIDTH, 100, color);
    }
}

void StartCountdown() {
    countdownStartTime = GetTime();
    isCountdown = true;
    isStartCD = false;
}

bool UpdateCountdown() {
    if (isCountdown) {
        float elapsedTime = GetTime() - countdownStartTime;
        counter = countDownTime - static_cast<int>(elapsedTime);

        if (counter <= 0) {
            
            //reset scores
            if (isGameOver)
                ResetScore();
            // turn off coundown, turn off gameover, turn back on countdownstarter
            isCountdown = false;
            isGameOver = false;
            isStartCD = true;

            return true;
        }  
        
    }

    return false;
}

void DrawCountdown() {
    if (isCountdown)
        DrawText(TextFormat("%d", counter), CENTER.x - 10, CENTER.y, 120, WHITE);
}

//Reset Functions
void ResetGame() {
    
    if (isStartCD)
     StartCountdown();
    countDownTime = 5;
    if (isGameOver) {
        if (redPoints > bluePoints)
            DisplayWinner(2, RED);
        else
            DisplayWinner(1, BLUE);
    }
}
void ResetRound() {
    // shorter cooldown, set it to oppisite side of point?
    countDownTime = 2;
    if (isStartCD)
        StartCountdown();
    
}

//Sounds
Music backgroundMusic;
Sound onHit;
Sound gameOver;
Sound scored;
void LoadMySounds() {
    InitAudioDevice();

    backgroundMusic = LoadMusicStream("Assets/BigPoppa.mp3");
    onHit = LoadSound("Assets/owa.mp3");
    gameOver = LoadSound("Assets/gameOver.mp3");
    scored = LoadSound("Assets/score.mp3");
    PlayMusicStream(backgroundMusic);
    SetMusicVolume(backgroundMusic, 0.2f);
    SetSoundVolume(onHit, 0.3);
}
void UnloadMySounds() {
    //unload audio
    UnloadMusicStream(backgroundMusic);
    UnloadSound(onHit);
    UnloadSound(gameOver);
    UnloadSound(scored);
    
    CloseAudioDevice();
}

int main()
{
    Vector2 ballPosition;
    Vector2 ballDirection;
    ResetBall(ballPosition, ballDirection);
    
    //Added ball speed increased everytime the ball hits the paddle. This mechanic makes sure that the rounds get increasingly
    //more difficult if the round is taking too long or is stale-mating.
   

    Vector2 paddle1Position, paddle2Position;
    paddle1Position.x = SCREEN_WIDTH * 0.05f;
    paddle2Position.x = SCREEN_WIDTH * 0.95f;
    paddle1Position.y = paddle2Position.y = CENTER.y;
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong");

    //Load textures
    Ball = LoadTexture("Assets/HappyBall.png");
    FastBall = LoadTexture("Assets/FastBall.png");
    BackgroundImg = LoadTexture("Assets/fatking.png");

    //load music
    LoadMySounds();

    SetTargetFPS(60);
    StartCountdown();
    countDownTime = 5;
    float hue = 0.0f;float hueball = 180.0f;
    
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        
        float ballDelta = BALL_SPEED * ballSpeedIncrease * dt;
        float paddleDelta = PADDLE_SPEED * dt;
        UpdateMusicStream(backgroundMusic);
        // Move paddle with key input
        if (IsKeyDown(KEY_W))
            paddle1Position.y -= paddleDelta;
        if (IsKeyDown(KEY_S))
            paddle1Position.y += paddleDelta;

        // Mirror paddle 1 for now
        paddle2Position.y = paddle1Position.y;

        float phh = PADDLE_HEIGHT * 0.5f;
        paddle1Position.y = Clamp(paddle1Position.y, phh, SCREEN_HEIGHT - phh);
        paddle2Position.y = Clamp(paddle2Position.y, phh, SCREEN_HEIGHT - phh);

        // Change the ball's direction on-collision
        Vector2 ballPositionNext = ballPosition + ballDirection * ballDelta;
        Circle ballBox = BallBox(ballPositionNext);
        Box paddle1Box = PaddleBox(paddle1Position);
        Box paddle2Box = PaddleBox(paddle2Position);
        // if they score
        if (ballBox.xMax > SCREEN_WIDTH) {
            //score point     
            bluePoints++;
            ResetBall(ballPosition, ballDirection);
            ResetRound();
            ballDirection.x *= -1.0f;
            PlaySound(scored);
        }       
        if (ballBox.xMin < 0.0f) {
            redPoints++;
            ResetBall(ballPosition, ballDirection);
            ResetRound();
            ballDirection.x *= -1.0f;      
            PlaySound(scored);
        }
        //if win condition
        if (bluePoints == 5 || redPoints == 5) { 
            PlaySound(gameOver);
            isGameOver = true;
            ResetBall(ballPosition, ballDirection);
            ResetGame();        
        }
        
        //hits screen bottoms
        if (ballBox.yMin < 0.0f || ballBox.yMax > SCREEN_HEIGHT) {
            ballDirection.y *= -1.0f;
            ChangeDirection(ballDirection);
        }
            

        // Update ball position after collision resolution, then render
        if (!isCountdown)
           ballPosition = ballPosition + ballDirection * ballDelta;
        
        //hit paddle
        if (BoxOverlap(ballBox, paddle1Box) || BoxOverlap(ballBox, paddle2Box)) {
            ballDirection.x *= -1;
            ballSpeedIncrease += 0.05;
            ChangeDirection(ballDirection);
            PlaySound(onHit);
        }
        
        BeginDrawing();

        hue += 1;
        if (hue > 360.0f) hue -= 360.0f;
        hueball += 1;
        if (hueball > 360.0f) hueball -= 360.0f;


        Color rainbowColor = ColorFromHSV(hue, 1.0f, 1.0f);
        Color rainbowColorBall = ColorFromHSV(hueball, 1.0f, 1.0f);

        
        ClearBackground(BLACK);
        DrawTexture(BackgroundImg, 0, 0, rainbowColorBall);
        DisplayScore( 80, bluePoints, redPoints); 

        if (ballSpeedIncrease > 1.4f)
        DrawBall(ballPosition, rainbowColor, FastBall);  
        else
        DrawBall(ballPosition, rainbowColor, Ball);  
        DrawCountdown();
        UpdateCountdown();
        DrawPaddle(paddle1Position, BLUE);
        DrawPaddle(paddle2Position, RED);
        EndDrawing();
    }
    UnloadTexture(Ball);
    UnloadTexture(FastBall);
    UnloadTexture(BackgroundImg);
    
    UnloadMySounds();
    CloseWindow();
    return 0;
}
