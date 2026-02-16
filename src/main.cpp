#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

struct TirePair {
    SDL_Point left;
    SDL_Point right;
};

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Proper Drift System",
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

    SDL_Surface* surface = SDL_CreateRGBSurface(0, 50, 25, 32, 0, 0, 0, 0);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));
    SDL_Texture* carTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    bool running = true;
    SDL_Event event;

    float posX = 400.0f;
    float posY = 300.0f;

    float velX = 0.0f;
    float velY = 0.0f;

    const float ACCELERATION = 2200.0f;
    const float MAX_SPEED = 650.0f;
    const float DRAG = 0.04f;
    const float GRIP = 0.04f;

    float angle = 0.0f;

    std::vector<TirePair> tireMarks;

    Uint64 lastCounter = SDL_GetPerformanceCounter();

    while (running)
    {
        // ---- DELTA TIME ----
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        float deltaTime =
            (float)(currentCounter - lastCounter) /
            SDL_GetPerformanceFrequency();
        lastCounter = currentCounter;

        // ---- EVENTS ----
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // ---- INPUT ----
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        float dirX = mouseX - posX;
        float dirY = mouseY - posY;
        float distance = std::sqrt(dirX * dirX + dirY * dirY);

        if (distance > 1.0f)
        {
            dirX /= distance;
            dirY /= distance;

            velX += dirX * ACCELERATION * deltaTime;
            velY += dirY * ACCELERATION * deltaTime;
        }

        // ---- DRAG ----
        velX -= velX * DRAG * deltaTime;
        velY -= velY * DRAG * deltaTime;

        // ---- LIMIT SPEED ----
        float speed = std::sqrt(velX * velX + velY * velY);
        if (speed > MAX_SPEED)
        {
            velX = (velX / speed) * MAX_SPEED;
            velY = (velY / speed) * MAX_SPEED;
        }

        // ---- ROTATION WITH WRAP ----
        if (speed > 5.0f)
        {
            float targetAngle = std::atan2(velY, velX);
            float diff = targetAngle - angle;

            while (diff > M_PI) diff -= 2 * M_PI;
            while (diff < -M_PI) diff += 2 * M_PI;

            float rotationSpeed = 6.0f;
            angle += diff * rotationSpeed * deltaTime;
        }

        // ---- DRIFT ----
        float forwardX = std::cos(angle);
        float forwardY = std::sin(angle);

        float forwardSpeed = velX * forwardX + velY * forwardY;

        float lateralX = velX - forwardX * forwardSpeed;
        float lateralY = velY - forwardY * forwardSpeed;

        lateralX -= lateralX * GRIP;
        lateralY -= lateralY * GRIP;

        velX = forwardX * forwardSpeed + lateralX;
        velY = forwardY * forwardSpeed + lateralY;

        // ---- TIRE MARKS (DUAL REAR) ----
        float lateralMagnitude =
            std::sqrt(lateralX * lateralX + lateralY * lateralY);

        if (lateralMagnitude > 120.0f)
        {
            float rearOffset = -22.0f;
            float tireSpread = 14.0f;

            float rearX = posX + std::cos(angle) * rearOffset;
            float rearY = posY + std::sin(angle) * rearOffset;

            float perpX = -std::sin(angle);
            float perpY =  std::cos(angle);

            TirePair pair;

            pair.left.x  = (int)(rearX + perpX * tireSpread);
            pair.left.y  = (int)(rearY + perpY * tireSpread);

            pair.right.x = (int)(rearX - perpX * tireSpread);
            pair.right.y = (int)(rearY - perpY * tireSpread);

            tireMarks.push_back(pair);

            if (tireMarks.size() > 1200)
                tireMarks.erase(tireMarks.begin());
        }

        // ---- UPDATE POSITION ----
        posX += velX * deltaTime;
        posY += velY * deltaTime;

        // ---- RENDER ----
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 35, 35, 35, 255);

        for (size_t i = 1; i < tireMarks.size(); ++i)
        {
            SDL_RenderDrawLine(renderer,
                tireMarks[i - 1].left.x,
                tireMarks[i - 1].left.y,
                tireMarks[i].left.x,
                tireMarks[i].left.y);

            SDL_RenderDrawLine(renderer,
                tireMarks[i - 1].right.x,
                tireMarks[i - 1].right.y,
                tireMarks[i].right.x,
                tireMarks[i].right.y);
        }

        SDL_Rect dstRect;
        dstRect.w = 50;
        dstRect.h = 25;
        dstRect.x = (int)(posX - 25);
        dstRect.y = (int)(posY - 12);

        double angleDegrees = angle * 180.0 / M_PI;

        SDL_RenderCopyEx(renderer,
            carTexture,
            NULL,
            &dstRect,
            angleDegrees,
            NULL,
            SDL_FLIP_NONE);

        SDL_RenderPresent(renderer);

        SDL_Delay(1);
    }

    SDL_DestroyTexture(carTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
