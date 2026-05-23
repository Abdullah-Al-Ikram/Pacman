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


bool canMove(float x, float y, int dir) {
    float margin = 0.2f;
    float nextX = x, nextY = y;

    if (dir == 0) nextX += margin + 0.1f;
    else if (dir == 1) nextY -= margin + 0.1f;
    else if (dir == 2) nextX -= margin + 0.1f;
    else if (dir == 3) nextY += margin + 0.1f;

    int gx = (int)(nextX + 0.5f);
    int gy = (int)(nextY + 0.5f);

    if (gx < 0 || gx >= MAZE_WIDTH || gy < 0 || gy >= MAZE_HEIGHT) return false;
    return maze[gy][gx] != 1;
}

void drawCircle(float cx, float cy, float r, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float angle = 2 * M_PI * i / segments;
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

void drawRectangle(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawBorderedBox(float x, float y, float w, float h) {
    glColor3f(0.2f, 0.2f, 0.4f);  // Dark blue fill
    drawRectangle(x, y, w, h);
    glColor3f(0.8f, 0.8f, 1.0f);  // Light blue border
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void drawPacman() {
    float px = OFFSET_X + pacman.x * CELL_SIZE + CELL_SIZE / 2;
    float py = OFFSET_Y + pacman.y * CELL_SIZE + CELL_SIZE / 2;

    glPushMatrix();
    glTranslatef(px, py, 0);
    if (pacman.direction != -1)
        glRotatef(-pacman.direction * 90, 0, 0, 1);

    // Main body - bright yellow
    glColor3f(1.0f, 0.95f, 0.0f);
    float start = pacman.mouthAngle * M_PI / 180.0f;
    float end = (360 - pacman.mouthAngle) * M_PI / 180.0f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (float a = start; a <= end; a += 0.08f) {
        glVertex2f(cos(a) * 11, sin(a) * 11);
    }
    glEnd();

    // Eye - black for contrast
    glColor3f(0, 0, 0);
    drawCircle(5, -3, 2, 8);
    glPopMatrix();
}

void drawGhost(struct Ghost* ghost) {
    float x = OFFSET_X + ghost->x * CELL_SIZE + CELL_SIZE / 2;
    float y = OFFSET_Y + ghost->y * CELL_SIZE + CELL_SIZE / 2;

    // Ghost body color
    switch (ghost->color) {
    case 0: glColor3f(1, 0, 0); break;           // Red - Blinky
    case 1: glColor3f(1, 0.4, 0.7); break;      // Pink - Pinky
    case 2: glColor3f(0.3, 1, 1); break;        // Cyan - Inky
    case 3: glColor3f(1, 0.6, 0.3); break;      // Orange - Clyde
    }

    // Main ghost body (rounded top)
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 180; i++) {
        float angle = (i / 180.0f) * M_PI;
        glVertex2f(x + cos(angle) * 11, y - sin(angle) * 11);
    }
    glEnd();

    // Ghost body (rectangular bottom)
    glColor3f(ghost->color == 0 ? 1 : (ghost->color == 1 ? 1 : (ghost->color == 2 ? 0.3f : 1)),
        ghost->color == 0 ? 0 : (ghost->color == 1 ? 0.4f : (ghost->color == 2 ? 1 : 0.6f)),
        ghost->color == 0 ? 0 : (ghost->color == 1 ? 0.7f : (ghost->color == 2 ? 1 : 0.3f)));
    drawRectangle(x - 11, y, 22, 10);

    // Ghost spikes/undulation
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 4; i++) {
        float spikeX = x - 8 + i * 5.5f;
        glVertex2f(spikeX, y + 10);
        glVertex2f(spikeX + 2, y + 13);
        glVertex2f(spikeX + 4, y + 10);
    }
    glEnd();

    // Eyes - white
    glColor3f(1, 1, 1);
    drawCircle(x - 4, y - 3, 3, 10);
    drawCircle(x + 4, y - 3, 3, 10);

    // Pupils - black
    glColor3f(0, 0, 0);
    drawCircle(x - 4, y - 3, 1.5f, 6);
    drawCircle(x + 4, y - 3, 1.5f, 6);
}

void drawMaze() {
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            float x = OFFSET_X + j * CELL_SIZE;
            float y = OFFSET_Y + i * CELL_SIZE;
            if (maze[i][j] == 1) {
                // Wall - gradient blue
                glColor3f(0.1f, 0.2f, 0.8f);
                drawRectangle(x, y, CELL_SIZE, CELL_SIZE);
                // Wall border for depth
                glColor3f(0.3f, 0.5f, 1.0f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(x, y);
                glVertex2f(x + CELL_SIZE, y);
                glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
                glVertex2f(x, y + CELL_SIZE);
                glEnd();
            }
            else if (maze[i][j] == 0) {
                // Pellet - white with glow effect
                glColor3f(1, 1, 1);
                drawCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 2.5f, 8);
                // Pellet glow
                glColor3f(0.5f, 0.5f, 0.5f);
                drawCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 3.5f, 8);
            }
        }
    }
}

void updatePacman() {
    if (pacman.direction == -1 && pacman.nextDirection == -1) return;

    if (pacman.nextDirection != -1 && canMove(pacman.x, pacman.y, pacman.nextDirection)) {
        pacman.direction = pacman.nextDirection;
    }

    if (canMove(pacman.x, pacman.y, pacman.direction)) {
        switch (pacman.direction) {
        case 0: pacman.x += pacman.speed; break;
        case 1: pacman.y -= pacman.speed; break;
        case 2: pacman.x -= pacman.speed; break;
        case 3: pacman.y += pacman.speed; break;
        }

        pacman.mouthAngle += pacman.mouthOpening * 5;
        if (pacman.mouthAngle >= 45 || pacman.mouthAngle <= 0) pacman.mouthOpening *= -1;
    }

    int gx = (int)(pacman.x + 0.5f);
    int gy = (int)(pacman.y + 0.5f);
    if (maze[gy][gx] == 0) {
        maze[gy][gx] = 2;
        pacman.score += 10;
        dotsEaten++;
        if (dotsEaten >= totalDots) currentState = WIN;
    }
}
