#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAZE_WIDTH 28
#define MAZE_HEIGHT 20
#define CELL_SIZE 26
#define OFFSET_X 30
#define OFFSET_Y 40
#define M_PI 3.14159265358979323846
#define HIGH_SCORES_FILE "highscores.txt"

enum GameState { MENU, PLAYING, PAUSED, GAME_OVER, WIN, HIGH_SCORES };
enum GameState currentState = MENU;
int menuSelection = 0;  // 0: Start, 1: High Scores, 2: Exit

struct Pacman {
    float x, y;
    float speed;
    int direction;      // 0: Right, 1: Up, 2: Left, 3: Down, -1: Idle
    int nextDirection;
    float mouthAngle;
    int mouthOpening;
    int lives;
    int score;
};

struct Ghost {
    float x, y;
    float speed;
    float baseSpeed;
    int direction;
    int color;
    float startX, startY;  // For resetting position
};

struct HighScore {
    int score;
    float time;
};

struct Pacman pacman;
struct Ghost ghosts[4];
int maze[MAZE_HEIGHT][MAZE_WIDTH];
int totalDots = 0;
int dotsEaten = 0;
float gameTime = 0;
float startTime = 0;
int elapsedSeconds = 0;
struct HighScore highScores[5] = { 0 };
int numHighScores = 0;