#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define MAX_BALLS 100
#define VEL_SCALE 3.0f

typedef struct {
    float x, y;
    float vx, vy;
    float rad;
    float mass;
    SDL_Color color;
} Ball;

void print_data(int numBalls, int numCollisions) {
    printf("DADOS DA SIMULAÇÃO\n");
    printf("Contador de Partículas: %d\n", numBalls);
    printf("Contador de Colisões: %d\n", numCollisions);
}

void render_ball(SDL_Renderer *renderer, Ball *ball)
{
    SDL_SetRenderDrawColor(renderer, ball->color.r, ball->color.g, ball->color.b, ball->color.a);
    int r = ball->rad;
    for (int w = 0; w < r * 2; w++) {
        for (int h = 0; h < r * 2; h++) {
            int dx = r - w;
            int dy = r - h;
            if (dx*dx + dy*dy <= r*r) {
                SDL_RenderDrawPoint(renderer, ball->x + dx, ball->y + dy);
            } 
        }
    }
}

void update_ball_position(Ball *ball, float dt)
{
    ball->x += ball->vx * dt;
    ball->y += ball->vy * dt;

    if (ball->x - ball->rad < 0 || ball->x + ball->rad > SCREEN_WIDTH) {
        ball->vx = -ball->vx;
        ball->x = ball->x < ball->rad ? ball->rad : SCREEN_WIDTH - ball->rad;
    }
    if (ball->y - ball->rad < 0 || ball->y + ball->rad > SCREEN_HEIGHT) {
        ball->vy = -ball->vy;
        ball->y = ball->y < ball->rad ? ball->rad : SCREEN_HEIGHT - ball->rad;
    }
}

void handle_collisions(Ball *balls, int numBalls, int *numCollisions)
{
    for (int i = 0; i < numBalls; i++) {
        for (int j = i + 1; j < numBalls; j++) {
            float dx = balls[j].x - balls[i].x;
            float dy = balls[j].y - balls[i].y;
            float distance = sqrt(dx*dx + dy*dy);

            if (distance <= balls[i].rad + balls[j].rad) {
                float nx = dx / distance;
                float ny = dy / distance;
                
                float p = 2 * (balls[i].vx * nx + balls[i].vy * ny - balls[j].vx * nx - balls[j].vy * ny) / (balls[i].mass + balls[j].mass);
                balls[i].vx = balls[i].vx - p * balls[j].mass * nx;
                balls[i].vy = balls[i].vy - p * balls[j].mass * ny;
                balls[j].vx = balls[j].vx + p * balls[i].mass * nx;
                balls[j].vy = balls[j].vy + p * balls[i].mass * ny;

                float overlap = 0.5f * (balls[i].rad + balls[j].rad - distance + 1.0f);
                balls[i].x -= overlap * nx;
                balls[i].y -= overlap * ny;
                balls[j].x += overlap * nx;
                balls[j].y += overlap * ny;

                (*numCollisions)++;
            }
        }
    }
}

void apply_friction(Ball *ball, float friction, float gravity)
{
    if (ball->vx != 0 || ball->vy != 0) {
        ball->vx *= (1.0f - friction);
        ball->vy *= (1.0f - friction);

        if (fabs(ball->vx) < 0.01f) ball->vx = 0;
        if (fabs(ball->vy) < 0.01f) ball->vy = 0;
    }
    ball->vy += gravity;
}

int main(int argc, char *argv[])
{
    float friction, gravity;
    printf("Atrito: ");
    scanf("%f", &friction);
    printf("Gravidade: ");
    scanf("%f", &gravity);
    printf("\n");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erro iniciando o SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Simulador de Colisões", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Erro iniciando a janela: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Erro iniciando o renderizador: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Ball balls[MAX_BALLS];
    int numBalls = 0;
    
    int running = 1;
    SDL_Event e;
    Uint32 lastTick = SDL_GetTicks(), currentTick;

    int numCollisions = 0;
    int dragging = 0;
    int startX = 0, startY = 0;
    int curX = 0, curY = 0;

    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if(e.type == SDL_QUIT) {
                running = 0;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                startX = e.button.x;
                startY = e.button.y;
                dragging = 1;
            }
            if (dragging) {
                curX = e.motion.x;
                curY = e.motion.y;
            }
            if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT && dragging) {
                int endX = e.button.x;
                int endY = e.button.y;

                float dx = endX - startX;
                float dy = endY - startY;
                float distance = sqrt(dx*dx + dy*dy);

                if (numBalls < MAX_BALLS) {
                    balls[numBalls].x = startX;
                    balls[numBalls].y = startY;
                    if (distance == 0) {
                        balls[numBalls].vx = 0;
                        balls[numBalls].vy = 0;    
                    } else {
                        balls[numBalls].vx = -1 * (dx / distance) * distance * VEL_SCALE;
                        balls[numBalls].vy = -1 * (dy / distance) * distance * VEL_SCALE;
                    }
                    balls[numBalls].mass = 1.0f;
                    balls[numBalls].rad = 20.0f;
                    balls[numBalls].color = (SDL_Color){rand() % 256, rand() % 256, rand() % 256, 255};
                    numBalls++;
                }

                dragging = 0;
            }
        }

        currentTick = SDL_GetTicks();
        float deltaTime = (currentTick - lastTick) / 1000.0f;
        lastTick = currentTick;

        for (int i = 0; i < numBalls; i++) {
            update_ball_position(&balls[i], deltaTime);
            apply_friction(&balls[i], friction, gravity);
        }

        handle_collisions(balls, numBalls, &numCollisions);

        print_data(numBalls, numCollisions);
        printf("\033[3A");

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (dragging) {
            float opX = 2*startX - curX;
            float opY = 2*startY - curY;
            
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(renderer, startX, startY, opX, opY);
        }

        for (int i = 0; i < numBalls; i++) {
            render_ball(renderer, &balls[i]);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    print_data(numBalls, numCollisions);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
