#include <SDL2/SDL.h>
#include <cmath>

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Mouse Follower",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    bool running = true;
    SDL_Event event;

    // Shape position (center of screen initially)
    float posX = 400.0f;
    float posY = 300.0f;

    int size = 20;      // Square size
    float speed = 0.05f; // How fast it follows mouse (0 to 1)

    while (running)
    {
        // ---- Handle Events ----
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // ---- Get Mouse Position ----
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // ---- Move Toward Mouse (Linear Interpolation) ----
        posX += (mouseX - posX) * speed;
        posY += (mouseY - posY) * speed;

        // ---- Render ----
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_Rect square;
        square.w = size;
        square.h = size;
        square.x = static_cast<int>(posX - size / 2);
        square.y = static_cast<int>(posY - size / 2);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &square);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
