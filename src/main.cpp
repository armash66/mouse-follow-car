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
        "Steering Drift System",
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
    const float STEER_SPEED = 4.0f;

    float angle = 0.0f;

    float driftTimer = 0.0f;
    const float DRIFT_INTERVAL = 0.02f; // seconds between skid marks

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

            // ---- STEERING ----
            float targetAngle = std::atan2(dirY, dirX);
            float diff = targetAngle - angle;

            while (diff > M_PI) diff -= 2 * M_PI;
            while (diff < -M_PI) diff += 2 * M_PI;

            angle += diff * STEER_SPEED * deltaTime;

            float forwardX = std::cos(angle);
            float forwardY = std::sin(angle);

            // ---- DISTANCE THROTTLE ----
            float throttleZone = 250.0f;
            float throttle = distance / throttleZone;

            if (throttle > 1.0f) throttle = 1.0f;
            if (throttle < 0.0f) throttle = 0.0f;

            // ---- ALIGNMENT THROTTLE ----
            float alignment = std::cos(diff);
            if (alignment < 0.0f)
                alignment = 0.0f;

            float finalThrottle = throttle * alignment;

            velX += forwardX * ACCELERATION * finalThrottle * deltaTime;
            velY += forwardY * ACCELERATION * finalThrottle * deltaTime;

            // ---- PROGRESSIVE BRAKE ----
            if (distance < 120.0f)
            {
                float brakeZone = 120.0f;
                float brakeIntensity = (brakeZone - distance) / brakeZone;
                float brakeStrength = 6.0f * brakeIntensity;

                velX -= velX * brakeStrength * deltaTime;
                velY -= velY * brakeStrength * deltaTime;
            }
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

        // ---- TIRE MARKS ----
        float lateralMagnitude =
        std::sqrt(lateralX * lateralX + lateralY * lateralY);

        // Drift detection threshold
        bool isDrifting = lateralMagnitude > 150.0f;

        if (isDrifting)
        {
            driftTimer += deltaTime;

            if (driftTimer >= DRIFT_INTERVAL)
            {
                driftTimer = 0.0f;

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

                if (tireMarks.size() > 1500)
                    tireMarks.erase(tireMarks.begin());
            }
        }
        else
        {
            driftTimer = 0.0f;
        }

        // ---- UPDATE POSITION ----
        posX += velX * deltaTime;
        posY += velY * deltaTime;

        // ---- RENDER ----
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 15, 15, 15, 255);

        for (size_t i = 1; i < tireMarks.size(); ++i)
        {
            // Left tire (thicker)
            SDL_RenderDrawLine(renderer,
                tireMarks[i - 1].left.x,
                tireMarks[i - 1].left.y,
                tireMarks[i].left.x,
                tireMarks[i].left.y);

            SDL_RenderDrawLine(renderer,
                tireMarks[i - 1].left.x + 1,
                tireMarks[i - 1].left.y,
                tireMarks[i].left.x + 1,
                tireMarks[i].left.y);

            // Right tire (thicker)
            SDL_RenderDrawLine(renderer,
                tireMarks[i - 1].right.x,
                tireMarks[i - 1].right.y,
                tireMarks[i].right.x,
                tireMarks[i].right.y);

            SDL_RenderDrawLine(renderer,
                tireMarks[i - 1].right.x + 1,
                tireMarks[i - 1].right.y,
                tireMarks[i].right.x + 1,
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
