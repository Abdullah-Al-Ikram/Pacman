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

void initMaze();
void initPacman();
void initGhosts();
void resetGame();
void loadHighScores();
void saveHighScores();
void addHighScore(int score, float time);

void drawText(float x, float y, const char* text, float scale = 1.0f) {
    glPushMatrix();
    glScalef(scale, scale, 1.0f);
    glRasterPos2f(x / scale, y / scale);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
    glPopMatrix();
}

void loadHighScores() {
    FILE* file = fopen(HIGH_SCORES_FILE, "r");
    numHighScores = 0;
    if (file) {
        while (numHighScores < 5 && fscanf(file, "%d %f", &highScores[numHighScores].score, &highScores[numHighScores].time) == 2) {
            numHighScores++;
        }
        fclose(file);
    }
}

void saveHighScores() {
    FILE* file = fopen(HIGH_SCORES_FILE, "w");
    if (file) {
        for (int i = 0; i < numHighScores; i++) {
            fprintf(file, "%d %.1f\n", highScores[i].score, highScores[i].time);
        }
        fclose(file);
    }
}

void addHighScore(int score, float time) {
    if (numHighScores < 5 || score > highScores[4].score) {
        highScores[numHighScores].score = score;
        highScores[numHighScores].time = time;
        if (numHighScores < 5) numHighScores++;

        // Sort high scores in descending order
        for (int i = 0; i < numHighScores - 1; i++) {
            for (int j = i + 1; j < numHighScores; j++) {
                if (highScores[i].score < highScores[j].score) {
                    struct HighScore temp = highScores[i];
                    highScores[i] = highScores[j];
                    highScores[j] = temp;
                }
            }
        }
        saveHighScores();
    }
}

void initMaze() {
    int layout[MAZE_HEIGHT][MAZE_WIDTH] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,0,1,1,1,1,1,2,2,2,2,1,1,1,1,1,0,1,1,1,1,1,1},
        {2,2,2,2,2,2,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,2,2,2,2,2,2},
        {1,1,1,1,1,1,0,1,1,2,2,2,2,2,2,2,2,2,2,1,1,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,1,1,1,1,1,2,2,2,2,1,1,1,1,1,0,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
        {1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1},
        {1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,1,1},
        {1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    totalDots = 0;
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            maze[i][j] = layout[i][j];
            if (maze[i][j] == 0) totalDots++;
        }
    }
}

void initPacman() {
    pacman.x = 1.0f;
    pacman.y = 1.0f;
    pacman.speed = 0.1f;
    pacman.direction = -1; // -1 means standing still
    pacman.nextDirection = -1;
    pacman.mouthAngle = 0;
    pacman.mouthOpening = 1;
    pacman.lives = 3;
    pacman.score = 0;
}

void initGhosts() {
    float startX[5] = { 13.0f, 14.0f, 13.0f, 14.0f };
    float startY[5] = { 8.0f, 8.0f, 9.0f, 9.0f };
    for (int i = 0; i < 4; i++) {
        ghosts[i].x = startX[i];
        ghosts[i].y = startY[i];
        ghosts[i].startX = startX[i];
        ghosts[i].startY = startY[i];
        ghosts[i].baseSpeed = 0.06f + (dotsEaten / (float)totalDots) * 0.04f;  // Speed increases with dots eaten
        ghosts[i].speed = ghosts[i].baseSpeed;
        ghosts[i].direction = rand() % 4;
        ghosts[i].color = i;
    }
}

void resetGame() {
    initMaze();
    initPacman();
    initGhosts();
    dotsEaten = 0;
    gameTime = 0;
    elapsedSeconds = 0;
    startTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}
