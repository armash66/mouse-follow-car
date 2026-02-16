#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>
#include <vector>

struct TirePair {
    SDL_Point left;
    SDL_Point right;
};

int main(int argc, char* argv[])
{
    // ---- INIT SDL ----
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        printf("SDL_image PNG init failed: %s\n", IMG_GetError());
        return 1;
    }

    // Disable texture filtering (pixel-perfect scaling)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    SDL_Window* window = SDL_CreateWindow(
        "Batmobile Drift System",
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

    // ---- LOAD IMAGE ----
    SDL_Texture* carTexture = IMG_LoadTexture(renderer, "../assets/batmobile.png");
    if (!carTexture)
    {
        printf("Failed to load batmobile.png: %s\n", IMG_GetError());
        return 1;
    }

    // ---- PHYSICS VARIABLES ----
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
    const float DRIFT_INTERVAL = 0.02f;

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

            float targetAngle = std::atan2(dirY, dirX);
            float diff = targetAngle - angle;

            while (diff > M_PI) diff -= 2 * M_PI;
            while (diff < -M_PI) diff += 2 * M_PI;

            angle += diff * STEER_SPEED * deltaTime;

            float forwardX = std::cos(angle);
            float forwardY = std::sin(angle);

            float throttleZone = 250.0f;
            float throttle = distance / throttleZone;
            if (throttle > 1.0f) throttle = 1.0f;
            if (throttle < 0.0f) throttle = 0.0f;

            float alignment = std::cos(diff);
            if (alignment < 0.0f)
                alignment = 0.0f;

            float finalThrottle = throttle * alignment;

            velX += forwardX * ACCELERATION * finalThrottle * deltaTime;
            velY += forwardY * ACCELERATION * finalThrottle * deltaTime;

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

        // ---- FORCE RENDER SIZE 28x14 ----
        SDL_Rect dstRect;
        dstRect.w = 112;
        dstRect.h = 56;
        dstRect.x = (int)(posX - 56);
        dstRect.y = (int)(posY - 28);

        double angleDegrees = angle * 180.0 / M_PI - 90.0;

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
    IMG_Quit();
    SDL_Quit();

    return 0;
}
