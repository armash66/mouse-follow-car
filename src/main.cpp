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

    SDL_Surface* surface = SDL_CreateRGBSurface(0, 40, 20, 32, 0, 0, 0, 0);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));
    SDL_Texture* carTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    bool running = true;
    SDL_Event event;

    // Shape position (center of screen initially)
    float posX = 400.0f;
    float posY = 300.0f;

    float velX = 0.0f;
    float velY = 0.0f;

    float accelerationStrength = 0.08f;
    float maxSpeed = 3.0f;
    float friction = 0.94f;

    float angle = 0.0f;

    int size = 20;      // Square size
    float movespeed = 0.07f; // How fast it follows mouse (0 to 1)

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

        float dirX = mouseX - posX;
        float dirY = mouseY - posY;

        float length = std::sqrt(dirX * dirX + dirY * dirY);

        if (length > 1.0f)
        {
            dirX /= length;
            dirY /= length;

            // Apply acceleration
            float distanceFactor = length / 300.0f;

            // Clamp it
            if (distanceFactor > 1.5f)
                distanceFactor = 1.5f;

            float chaseForce = accelerationStrength * distanceFactor;

            velX += dirX * chaseForce;
            velY += dirY * chaseForce;
        }

        // Apply friction
        velX *= friction;
        velY *= friction;

        // Calculate forward vector from angle
        float forwardX = std::cos(angle);
        float forwardY = std::sin(angle);

        // Project velocity onto forward direction
        float forwardSpeed = velX * forwardX + velY * forwardY;

        // Rebuild velocity using mostly forward component
        float driftFactor = 0.08f;  // lower = more drift

        velX = forwardX * forwardSpeed + (velX - forwardX * forwardSpeed) * driftFactor;
        velY = forwardY * forwardSpeed + (velY - forwardY * forwardSpeed) * driftFactor;

        // Limit max speed
        float speed = std::sqrt(velX * velX + velY * velY);

        if (std::abs(velX) > 0.01f || std::abs(velY) > 0.01f)
        {
            angle = std::atan2(velY, velX);
        }

        if (speed > maxSpeed)
        {
            velX = (velX / speed) * maxSpeed;
            velY = (velY / speed) * maxSpeed;
        }

        // Update position
        posX += velX;
        posY += velY;

        // ---- Render ----
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_Rect dstRect;
        dstRect.w = 40;
        dstRect.h = 20;
        dstRect.x = static_cast<int>(posX - dstRect.w / 2);
        dstRect.y = static_cast<int>(posY - dstRect.h / 2);

        double angleDegrees = angle * 180.0 / M_PI;

        SDL_RenderCopyEx(renderer, carTexture, NULL, &dstRect, angleDegrees, NULL, SDL_FLIP_NONE);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(carTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
