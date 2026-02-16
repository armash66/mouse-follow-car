#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Mouse Drift Chase",
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

    // Create simple car texture
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 40, 20, 32, 0, 0, 0, 0);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));
    SDL_Texture* carTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    bool running = true;
    SDL_Event event;

    // Position
    float posX = 400.0f;
    float posY = 300.0f;

    // Velocity
    float velX = 0.0f;
    float velY = 0.0f;

    // Physics tuning
    float accelerationStrength = 0.08f;
    float maxSpeed = 3.5f;
    float friction = 0.96f;
    float lateralDamping = 0.99f; // extreme drift

    float angle = 0.0f;

    std::vector<SDL_Point> tireMarks;

    while (running)
    {
        // ---- EVENTS ----
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // ---- INPUT ----
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // ---- ACCELERATION ----
        float dirX = mouseX - posX;
        float dirY = mouseY - posY;

        float length = std::sqrt(dirX * dirX + dirY * dirY);

        if (length > 1.0f)
        {
            dirX /= length;
            dirY /= length;

            float distanceFactor = length / 300.0f;
            if (distanceFactor > 1.5f)
                distanceFactor = 1.5f;

            float chaseForce = accelerationStrength * distanceFactor;

            velX += dirX * chaseForce;
            velY += dirY * chaseForce;
        }

        // ---- FRICTION ----
        velX *= friction;
        velY *= friction;

        // ---- LIMIT SPEED ----
        float speed = std::sqrt(velX * velX + velY * velY);
        if (speed > maxSpeed)
        {
            velX = (velX / speed) * maxSpeed;
            velY = (velY / speed) * maxSpeed;
        }

        // ---- UPDATE ANGLE ----
        if (std::abs(velX) > 0.01f || std::abs(velY) > 0.01f)
        {
            angle = std::atan2(velY, velX);
        }

        // ---- DRIFT CALCULATION ----
        float forwardX = std::cos(angle);
        float forwardY = std::sin(angle);

        float forwardSpeed = velX * forwardX + velY * forwardY;

        float lateralX = velX - forwardX * forwardSpeed;
        float lateralY = velY - forwardY * forwardSpeed;

        lateralX *= lateralDamping;
        lateralY *= lateralDamping;

        velX = forwardX * forwardSpeed + lateralX;
        velY = forwardY * forwardSpeed + lateralY;

        // ---- TIRE MARKS ----
        float lateralMagnitude = std::sqrt(lateralX * lateralX + lateralY * lateralY);

        if (lateralMagnitude > 0.6f)
        {
            tireMarks.push_back({ static_cast<int>(posX), static_cast<int>(posY) });

            if (tireMarks.size() > 800)
                tireMarks.erase(tireMarks.begin());
        }

        // ---- UPDATE POSITION ----
        posX += velX;
        posY += velY;

        // ---- RENDER ----
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        // Draw tire trails (continuous lines)
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);

        for (size_t i = 1; i < tireMarks.size(); ++i)
        {
            SDL_RenderDrawLine(renderer,
                tireMarks[i - 1].x,
                tireMarks[i - 1].y,
                tireMarks[i].x,
                tireMarks[i].y);
        }

        // Draw rotated car
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
