#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <SDL3/SDL.h>
#include "rectangle.h"
#include "circle.h"

using namespace std;

//define window dimensions
constexpr int WINDOW_WIDTH{ 700 };
constexpr int WINDOW_HEIGHT{ 800 };

// Vertical scrolling grid related variables
constexpr float VERTICAL_SCROLLING_SPEED{ 100 };
constexpr int VERTICAL_SCROLLING_GRID_SQUARE_SIZE{ 20 };
vector<Rectangle> scrollingGrid;

// Animation related variables
float DT{ 0.01f };
Uint64 startTime{ 0 };
Uint64 endTime{ 0 };

//define SDL Window related variables
SDL_Window* window{ nullptr };
SDL_Renderer* renderer{ nullptr };
SDL_Event currentEvent;
SDL_Color backgroundColor{ 255, 255, 255, 255 };

bool quit{ false };

float mouseX{ -1.0f }, mouseY{ -1.0f };

float displayScale{ 1.0f };


Rectangle player;
vector<Rectangle> enemies;
vector<Circle> playerBullets;
vector<Circle> enemyBullets;

bool leftMouseHeld = false;
float playerShootTimer = 0.0f;
float enemyShootTimer = 0.0f;
float totalPlayTime = 0.0f;


int gameStatus = 0;

void initScollingRectangles() {
    scrollingGrid.clear();
    int numGridRectangles = (WINDOW_HEIGHT / VERTICAL_SCROLLING_GRID_SQUARE_SIZE) / 2 + 2;

    for (int i = 0; i < numGridRectangles; i++) {
        Rectangle newGridRectangleLeft, newGridRectangleRight;

        newGridRectangleLeft.width = newGridRectangleLeft.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleLeft.pos.x = newGridRectangleLeft.width / 2.0f;
        newGridRectangleLeft.pos.y = WINDOW_HEIGHT - newGridRectangleLeft.height / 2.0f - 2.0f * i * newGridRectangleLeft.height;
        newGridRectangleLeft.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleLeft.color = SDL_Color{ 235, 235, 235, 255 };

        newGridRectangleRight.width = newGridRectangleRight.height = VERTICAL_SCROLLING_GRID_SQUARE_SIZE;
        newGridRectangleRight.pos.x = WINDOW_WIDTH - newGridRectangleRight.width / 2.0f;
        newGridRectangleRight.pos.y = WINDOW_HEIGHT - newGridRectangleRight.height / 2.0f - 2.0f * i * newGridRectangleRight.height;
        newGridRectangleRight.vel = glm::vec2(0.0f, VERTICAL_SCROLLING_SPEED);
        newGridRectangleRight.color = SDL_Color{ 235, 235, 235, 255 };

        scrollingGrid.push_back(newGridRectangleLeft);
        scrollingGrid.push_back(newGridRectangleRight);
    }
}

void initGame() {
    gameStatus = 0;
    leftMouseHeld = false;
    playerShootTimer = 0.0f;
    enemyShootTimer = 0.0f;
    totalPlayTime = 0.0f;

    enemies.clear();
    playerBullets.clear();
    enemyBullets.clear();

    initScollingRectangles();

    player.width = 50.0f;
    player.height = 50.0f;
    player.pos.x = WINDOW_WIDTH / 2.0f;
    player.pos.y = WINDOW_HEIGHT - player.height / 2.0f;
    player.color = SDL_Color{ 0, 200, 0, 255 };

    
    float sliceWidth = WINDOW_WIDTH / 3.0f;
    for (int i = 0; i < 3; i++) {
        Rectangle e;
        e.width = 50.0f;
        e.height = 50.0f;
        e.color = SDL_Color{ 255, 165, 0, 255 };

        int minX = (int)(i * sliceWidth + e.width / 2.0f);
        int maxX = (int)((i + 1) * sliceWidth - e.width / 2.0f);
        int minY = (int)(e.height / 2.0f);
        int maxY = (int)(WINDOW_HEIGHT / 2.0f - e.height / 2.0f);

        e.pos.x = (float)(minX + rand() % (maxX - minX + 1));
        e.pos.y = (float)(minY + rand() % (maxY - minY + 1));

        enemies.push_back(e);
    }

    SDL_SetWindowRelativeMouseMode(window, true);
}

bool hitCircleRect(Circle c, Rectangle r) {
    float closestX = c.pos.x;
    float closestY = c.pos.y;

    if (c.pos.x < r.pos.x - r.width / 2.0f) closestX = r.pos.x - r.width / 2.0f;
    else if (c.pos.x > r.pos.x + r.width / 2.0f) closestX = r.pos.x + r.width / 2.0f;

    if (c.pos.y < r.pos.y - r.height / 2.0f) closestY = r.pos.y - r.height / 2.0f;
    else if (c.pos.y > r.pos.y + r.height / 2.0f) closestY = r.pos.y + r.height / 2.0f;

    float dx = c.pos.x - closestX;
    float dy = c.pos.y - closestY;
    return (dx * dx + dy * dy) <= (c.radius * c.radius);
}

bool hitRectRect(Rectangle a, Rectangle b) {
    return abs(a.pos.x - b.pos.x) < (a.width / 2.0f + b.width / 2.0f) &&
        abs(a.pos.y - b.pos.y) < (a.height / 2.0f + b.height / 2.0f);
}

bool initWindow() {
    bool success{ true };

    //Try to initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s\n", SDL_GetError());
        success = false;
    }
    else {
        //Try to create the window and renderer
        displayScale = SDL_GetDisplayContentScale(1);

        if (!SDL_CreateWindowAndRenderer(
            "SDL Hello World Example",
            static_cast<int>(displayScale * WINDOW_WIDTH),
            static_cast<int>(displayScale * WINDOW_HEIGHT),
            0,
            &window, &renderer)) {
            SDL_Log("Failed to create window and renderer: %s\n", SDL_GetError());
            success = false;
        }
        else {
            //Apply global display scaling to renderer
            SDL_SetRenderScale(renderer, displayScale, displayScale);

            //Set background color
            SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b,
                backgroundColor.a);

            //Apply background color
            SDL_RenderClear(renderer);
        }
    }

    return success;
}

void processEvents() {
    //Check for events in queue
    while (SDL_PollEvent(&currentEvent)) {
        //User requests quit
        if (currentEvent.type == SDL_EVENT_QUIT) {
            quit = true;
        }

        //Mouse event -> pressed button
        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT && gameStatus == 0) {
                leftMouseHeld = true;
                playerShootTimer = 0.0f;

                Circle b(10.0f, player.pos, SDL_Color{ 255, 0, 0, 255 });
                b.vel = glm::vec2(0.0f, -500.0f);
                playerBullets.push_back(b);
            }
        }

        if (currentEvent.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (currentEvent.button.button == SDL_BUTTON_LEFT) {
                leftMouseHeld = false;
            }
        }

        //Mouse event -> mouse movement
        if (currentEvent.type == SDL_EVENT_MOUSE_MOTION) {
            if (gameStatus == 0) {
                player.pos.x += currentEvent.motion.xrel / displayScale;
                player.pos.y += currentEvent.motion.yrel / displayScale;

                if (player.pos.x < player.width / 2.0f) player.pos.x = player.width / 2.0f;
                if (player.pos.x > WINDOW_WIDTH - player.width / 2.0f) player.pos.x = WINDOW_WIDTH - player.width / 2.0f;
                if (player.pos.y < player.height / 2.0f) player.pos.y = player.height / 2.0f;
                if (player.pos.y > WINDOW_HEIGHT - player.height / 2.0f) player.pos.y = WINDOW_HEIGHT - player.height / 2.0f;
            }
        }

        //Keyboard event
        if (currentEvent.type == SDL_EVENT_KEY_DOWN) {
            switch (currentEvent.key.key) {
            case SDLK_R:
                if (gameStatus != 0) {
                    initGame();
                }
                break;
            case SDLK_ESCAPE:
                quit = true;
                break;
            default:
                break;
            }
        }
    }
}

void animate() {
    if (gameStatus != 0) {
        return;
    }

    totalPlayTime += DT;

    // Scrolling grid
    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);

        gridRectangle.pos += gridRectangle.vel * DT;

        if (gridRectangle.pos.y > (WINDOW_HEIGHT + gridRectangle.height / 2.0f)) {
            gridRectangle.pos.y -= gridRectangle.height * scrollingGrid.size();
        }
    }

    
    if (leftMouseHeld) {
        playerShootTimer += DT;
        if (playerShootTimer >= 0.3f) {
            playerShootTimer -= 0.3f;
            Circle b(10.0f, player.pos, SDL_Color{ 255, 0, 0, 255 });
            b.vel = glm::vec2(0.0f, -500.0f);
            playerBullets.push_back(b);
        }
    }

    
    if (totalPlayTime >= 1.0f) {
        enemyShootTimer += DT;
        if (enemyShootTimer >= 3.0f) {
            enemyShootTimer -= 3.0f;
            for (int i = 0; i < enemies.size(); i++) {
                glm::vec2 dir = player.pos - enemies[i].pos;
                float dist = sqrt(dir.x * dir.x + dir.y * dir.y);

                Circle b(10.0f, enemies[i].pos, SDL_Color{ 255, 0, 255, 255 });
                if (dist > 0.0f) {
                    b.vel = (dir / dist) * 100.0f; // 5x slower than 500
                }
                else {
                    b.vel = glm::vec2(0.0f, 100.0f);
                }
                enemyBullets.push_back(b);
            }
        }
    }
        
    for (int i = 0; i < playerBullets.size(); i++) {
        playerBullets[i].pos += playerBullets[i].vel * DT;
        if (playerBullets[i].pos.y < -playerBullets[i].radius) {
            playerBullets.erase(playerBullets.begin() + i);
            i--;
        }
    }

        for (int i = 0; i < enemyBullets.size(); i++) {
        enemyBullets[i].pos += enemyBullets[i].vel * DT;
        if (enemyBullets[i].pos.x < -enemyBullets[i].radius || enemyBullets[i].pos.x > WINDOW_WIDTH + enemyBullets[i].radius ||
            enemyBullets[i].pos.y < -enemyBullets[i].radius || enemyBullets[i].pos.y > WINDOW_HEIGHT + enemyBullets[i].radius) {
            enemyBullets.erase(enemyBullets.begin() + i);
            i--;
        }
    }

        for (int i = 0; i < playerBullets.size(); i++) {
        for (int j = 0; j < enemies.size(); j++) {
            if (hitCircleRect(playerBullets[i], enemies[j])) {
                playerBullets.erase(playerBullets.begin() + i);
                enemies.erase(enemies.begin() + j);
                i--;
                break;
            }
        }
    }

    if (enemies.empty()) {
        gameStatus = 1;
        player.color = SDL_Color{ 0, 0, 255, 255 };
        enemies.clear();
        playerBullets.clear();
        enemyBullets.clear();
        SDL_SetWindowRelativeMouseMode(window, false);
        return;
    }

    for (int i = 0; i < enemies.size(); i++) {
        if (hitRectRect(player, enemies[i])) {
            gameStatus = 2;
            player.color = SDL_Color{ 255, 0, 0, 255 };
            enemies.clear();
            playerBullets.clear();
            enemyBullets.clear();
            SDL_SetWindowRelativeMouseMode(window, false);
            return;
        }
    }

    
    for (int i = 0; i < enemyBullets.size(); i++) {
        if (hitCircleRect(enemyBullets[i], player)) {
            gameStatus = 2;
            player.color = SDL_Color{ 255, 0, 0, 255 };
            enemies.clear();
            playerBullets.clear();
            enemyBullets.clear();
            SDL_SetWindowRelativeMouseMode(window, false);
            return;
        }
    }
}

void drawFrame() {
    //Clear the background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);

    //Draw scrolling grid
    for (int i = 0; i < scrollingGrid.size(); i++) {
        Rectangle& gridRectangle = scrollingGrid.at(i);
        gridRectangle.draw(renderer);
    }

    //Draw scene
    if (gameStatus == 0) {
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].draw(renderer);
        }
        for (int i = 0; i < playerBullets.size(); i++) {
            playerBullets[i].draw(renderer);
        }
        for (int i = 0; i < enemyBullets.size(); i++) {
            enemyBullets[i].draw(renderer);
        }
    }

    player.draw(renderer);

    //Update window
    SDL_RenderPresent(renderer);
}

void cleanup() {
    //Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    //Destroy window
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    //Quit SDL
    SDL_Quit();
}

int main() {
    srand((unsigned int)time(NULL));

    //Initialize window
    if (!initWindow()) {
        cout << "Failed to initialize" << endl;
        return -1;
    }

    SDL_zero(currentEvent);

    initGame();

    //Game loop
    while (!quit) {
        startTime = SDL_GetTicks();

        processEvents();
        animate();
        drawFrame();

        endTime = SDL_GetTicks();
        DT = (endTime - startTime) / 1000.0f;
    }

    cleanup();
    return 0;
}