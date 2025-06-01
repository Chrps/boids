#include <SDL.h>
#include <SDL_ttf.h>
#include <cstdlib>   // for rand()
#include <ctime> 
#include <vector>
#include "slider.hpp"
#include "font.hpp"
#include "toggle.hpp"
#include "text.hpp"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int window_width = 1800;
int window_height = 900;
bool running = true;
bool follow_mouse = true;
int mouseX = 0, mouseY = 0;
float mouseFollowFactor = 0.1f;
int mouseFollowRange = 200;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

int numBoids = 250;
int numBoidsSliderValue = numBoids; // Slider value for number of boids
int visualRange = 75;
int minAvoidanceDistance = 30;
float centeringFactor = 0.001f;
float avoidanceFactor = 0.04f;
float matchingFactor = 0.05f;
float speedLimit = 3.5f;
int margin = 0;
int turnFactor = 4;
int simulationDelayMs = 0; // milliseconds

int slider_separation = 50;
int visualRange_y = 25;
int minAvoidanceDistance_y = visualRange_y + slider_separation;
int speedLimit_y = minAvoidanceDistance_y + slider_separation;
int centeringFactor_y = speedLimit_y + slider_separation;
int avoidanceFactor_y = centeringFactor_y + slider_separation;
int matchingFactor_y = avoidanceFactor_y + slider_separation;
int simulationDelay_y = matchingFactor_y + slider_separation;
int numBoidsSlider_y = simulationDelay_y + slider_separation;
int followMouseToggle_y = numBoidsSlider_y + slider_separation;


Slider visualRangeSlider = { 20, visualRange_y, 200, 20, SliderType::INT, {.intValue = &visualRange}, 1.0f, 200.0f, "Visual Range" };
Slider minAvoidanceSlider = { 20, minAvoidanceDistance_y, 200, 20, SliderType::INT, {.intValue = &minAvoidanceDistance}, 1.0f, 40.0f, "Min Avoidance Distance" };
Slider speedLimitSlider = { 20, speedLimit_y, 200, 20, SliderType::FLOAT, {.floatValue = &speedLimit}, 0.1f, 5.0f, "Speed Limit" };
Slider centeringFactorSlider = { 20, centeringFactor_y, 200, 20, SliderType::FLOAT, {.floatValue = &centeringFactor}, 0.001f, 0.01f, "Centering Factor" };
Slider avoidanceFactorSlider = { 20, avoidanceFactor_y, 200, 20, SliderType::FLOAT, {.floatValue = &avoidanceFactor}, 0.001f, 0.1f, "Avoidance Factor" };
Slider matchingFactorSlider = { 20, matchingFactor_y, 200, 20, SliderType::FLOAT, {.floatValue = &matchingFactor}, 0.001f, 0.1f, "Matching Factor" };
Slider simulationDelaySlider = { 20, simulationDelay_y, 200, 20, SliderType::INT, {.intValue = &simulationDelayMs}, 0, 50, "Simulation Delay (ms)" };
Slider numBoidsSlider = {20, numBoidsSlider_y, 200, 20, SliderType::INT, {.intValue = &numBoidsSliderValue}, 1, 500, "Number of Boids"};
Toggle followMouseToggle = { 20, followMouseToggle_y, 50, 50, &follow_mouse, "Follow Mouse"};

struct Boid {
    float x, y;
    float vx, vy;
};

struct BoidCenter {
    float x, y;
};

std::vector<Boid> boids;

// give each boid a random initial position and velocity
void init_boids() {
    boids.clear();
    boids.reserve(numBoids); // avoid reallocations
    for (int i = 0; i < numBoids; ++i) {
        Boid b;
        b.x = rand() % window_width;
        b.y = rand() % window_height;
        b.vx = (rand() % 200 - 100) / 100.0f;
        b.vy = (rand() % 200 - 100) / 100.0f;
        boids.push_back(b);
    }
}

BoidCenter calculate_center(const Boid& boid) {
    BoidCenter center = {0, 0};
    int count = 0;

    // Calculate the center of mass of the boids within the visual range not including the boid itself
    for (const auto& other : boids) {
        if (&other != &boid) {
            float dx = other.x - boid.x;
            float dy = other.y - boid.y;
            if (dx * dx + dy * dy < visualRange * visualRange) {
                center.x += other.x;
                center.y += other.y;
                count++;
            }
        }
    }
    // calculate the average position
    // If no other boids are within the visual range, return the boid's own position
    if (count > 0) {
        center.x /= count;
        center.y /= count;
    } else {
        center.x = boid.x;
        center.y = boid.y;
    }

    return center;
}

void fly_towards_center(Boid& boid) {
    BoidCenter center = calculate_center(boid);
    boid.vx += (center.x - boid.x) * centeringFactor;
    boid.vy += (center.y - boid.y) * centeringFactor;
}

void fly_towards_mouse(Boid& boid, int mouseX, int mouseY) {
    // Calculate the direction towards the mouse position
    float dx = mouseX - boid.x;
    float dy = mouseY - boid.y;
    float distSq = dx * dx + dy * dy;

    // Only apply force if within visual range
    if (distSq < mouseFollowRange * mouseFollowRange && distSq > 0.0001f) {
        float dist = sqrt(distSq);
        // Normalize the direction vector and scale by the follow factor
        dx /= dist;
        dy /= dist;
        boid.vx += dx * mouseFollowFactor;
        boid.vy += dy * mouseFollowFactor;
    }
}

void avoid_other_boids(Boid& boid) {
    for (const auto& other : boids) {
        if (&other != &boid) {
            float dx = other.x - boid.x;
            float dy = other.y - boid.y;
            float distSq = dx * dx + dy * dy;
            float minDistSq = minAvoidanceDistance * minAvoidanceDistance;

            if (distSq < minDistSq && distSq > 0.0001f) {
                float dist = sqrt(distSq);
                // stronger avoidance force when closer (inverse proportional)
                float force = avoidanceFactor * (minAvoidanceDistance - dist) / dist;
                boid.vx -= dx * force;
                boid.vy -= dy * force;
            }
        }
    }
}

void match_velocity(Boid& boid) {
    BoidCenter center = calculate_center(boid);
    float avgVx = 0, avgVy = 0;
    int count = 0;

    // Calculate the average velocity of the boids within the visual range not including the boid itself
    for (const auto& other_boid : boids) {
        if (&other_boid != &boid) {
            float dx = other_boid.x - boid.x;
            float dy = other_boid.y - boid.y;
            if (dx * dx + dy * dy < visualRange * visualRange) {
                avgVx += other_boid.vx;
                avgVy += other_boid.vy;
                count++;
            }
        }
    }
    // If there are other boids in range, adjust velocity towards their average
    if (count > 0) {
        avgVx /= count;
        avgVy /= count;
        boid.vx += (avgVx - boid.vx) * matchingFactor;
        boid.vy += (avgVy - boid.vy) * matchingFactor;
    }
}

void limit_speed(Boid& boid) {
    float speed = sqrt(boid.vx * boid.vx + boid.vy * boid.vy);
    if (speed > speedLimit) {
        boid.vx = (boid.vx / speed) * speedLimit;
        boid.vy = (boid.vy / speed) * speedLimit;
    }
}

void keep_boid_in_bounds(Boid& boid) {
    if (boid.x < margin) {
        boid.vx += turnFactor;
    }
    if (boid.x > window_width - margin) {
        boid.vx -= turnFactor;
    }
    if (boid.y < margin) {
        boid.vy += turnFactor;
    }
    if (boid.y > window_height - margin) {
        boid.vy -= turnFactor;
    }
}

void update_boids() {
    static Uint32 lastUpdate = 0;
    Uint32 now = SDL_GetTicks();

    if (now - lastUpdate < simulationDelayMs) {
        // Skip updating boids if delay not elapsed
        return;
    }
    lastUpdate = now;
    for (auto& boid : boids) {

        // Apply flocking behaviors
        fly_towards_center(boid);
        if (follow_mouse == true) {
            fly_towards_mouse(boid, mouseX, mouseY); 
        }
        avoid_other_boids(boid);
        match_velocity(boid);
        limit_speed(boid);
        keep_boid_in_bounds(boid);

        // Update position
        boid.x += boid.vx;
        boid.y += boid.vy;
    }
}

inline float fast_inv_sqrt(float x) {
    union { float f; uint32_t i; } conv = { .f = x };
    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= 1.5f - (x * 0.5f * conv.f * conv.f);
    return conv.f;
}

// Fast square root approximation
inline float fast_sqrt(float x) {
    return x * fast_inv_sqrt(x);
}


void update_boid_fast(Boid& boid, int mouseX, int mouseY) {
    float centerX = 0, centerY = 0;
    float avgVx = 0, avgVy = 0;
    float avoidX = 0, avoidY = 0;
    int neighborCount = 0;

    const float visualRangeSq = visualRange * visualRange;
    const float minAvoidanceDistSq = minAvoidanceDistance * minAvoidanceDistance;

    for (const auto& other : boids) {
        if (&other == &boid) continue;

        float ox = other.x;
        float oy = other.y;
        float ovx = other.vx;
        float ovy = other.vy;

        float dx = ox - boid.x;
        float dy = oy - boid.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < visualRangeSq) {
            // Centering
            centerX += ox;
            centerY += oy;

            // Velocity matching
            avgVx += ovx;
            avgVy += ovy;

            neighborCount++;

            // Avoidance
            if (distSq < minAvoidanceDistSq && distSq > 0.0001f) {
                float dist = fast_sqrt(distSq);
                float force = avoidanceFactor * (minAvoidanceDistance - dist) / dist;
                avoidX -= dx * force;
                avoidY -= dy * force;
            }
        }
    }


    // Apply centering and velocity matching if neighbors found
    if (neighborCount > 0) {
        centerX /= neighborCount;
        centerY /= neighborCount;
        avgVx /= neighborCount;
        avgVy /= neighborCount;

        boid.vx += (centerX - boid.x) * centeringFactor;
        boid.vy += (centerY - boid.y) * centeringFactor;

        boid.vx += (avgVx - boid.vx) * matchingFactor;
        boid.vy += (avgVy - boid.vy) * matchingFactor;
    }

    // Apply avoidance force
    boid.vx += avoidX;
    boid.vy += avoidY;

    // Mouse following (separate range check)
    if (follow_mouse)  // Only apply mouse following if enabled
    {
        float dx = mouseX - boid.x;
        float dy = mouseY - boid.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < mouseFollowRange * mouseFollowRange && distSq > 0.0001f) {
            float dist = fast_sqrt(distSq);
            dx /= dist;
            dy /= dist;
            boid.vx += dx * mouseFollowFactor;
            boid.vy += dy * mouseFollowFactor;
        }
    }

    // Limit speed
    {
        float speed = fast_sqrt(boid.vx * boid.vx + boid.vy * boid.vy);
        if (speed > speedLimit) {
            boid.vx = (boid.vx / speed) * speedLimit;
            boid.vy = (boid.vy / speed) * speedLimit;
        }
    }

    // Keep inside bounds
    if (boid.x < margin)        boid.vx += turnFactor;
    else if (boid.x > window_width - margin)  boid.vx -= turnFactor;

    if (boid.y < margin)        boid.vy += turnFactor;
    else if (boid.y > window_height - margin) boid.vy -= turnFactor;
}

void update_boids_fast() {
    static Uint32 lastUpdate = 0;
    Uint32 now = SDL_GetTicks();

    if (now - lastUpdate < simulationDelayMs) {
        // Skip updating boids if delay not elapsed
        return;
    }
    lastUpdate = now;

    for (auto& boid : boids) {
        update_boid_fast(boid, mouseX, mouseY);
        // Update position
        boid.x += boid.vx;
        boid.y += boid.vy;
    }
}

void render_boids_as_rects() {
    for (const auto& boid : boids) {
        int neighborCount = 0;

        for (const auto& other : boids) {
            if (&boid != &other) {
                float dx = other.x - boid.x;
                float dy = other.y - boid.y;
                if (dx*dx + dy*dy < visualRange * visualRange) {
                    neighborCount++;
                }
            }
        }

        // Map neighborCount to color (0 = blue, maxNeighbors = red)
        // Clamp maxNeighbors to, say, 20 for color scaling
        const int maxNeighbors = 30;
        float t = std::min(neighborCount, maxNeighbors) / static_cast<float>(maxNeighbors);

        Uint8 r = static_cast<Uint8>(t * 255);
        Uint8 g = 0;
        Uint8 b = static_cast<Uint8>((1.0f - t) * 255);

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);

        SDL_Rect rect = { static_cast<int>(boid.x), static_cast<int>(boid.y), 5, 5 };
        SDL_RenderFillRect(renderer, &rect);
    }
}

void render_boids_as_triangles() {
    // render boids as triangles pointing in the direction of their velocity but not scaled
    for (const auto& boid : boids) {
        // Triangle in local space (pointing up)
        float size = 10.0f;
        float halfSize = size / 2.0f;
        float angle = atan2(boid.vy, boid.vx);
        SDL_Point points[4];

        float cosA = cos(angle);
        float sinA = sin(angle);
        int neighborCount = 0;
        // Set color based on velocity
        for (const auto& other : boids) {
            if (&boid != &other) {
                float dx = other.x - boid.x;
                float dy = other.y - boid.y;
                if (dx*dx + dy*dy < visualRange * visualRange) {
                    neighborCount++;
                }
            }
        }

        // Map neighborCount to color (0 = blue, maxNeighbors = red)
        // Clamp maxNeighbors to, say, 20 for color scaling
        const int maxNeighbors = 30;
        float t = std::min(neighborCount, maxNeighbors) / static_cast<float>(maxNeighbors);

        Uint8 r = static_cast<Uint8>(t * 255);
        Uint8 g = 0;
        Uint8 b = static_cast<Uint8>((1.0f - t) * 255);


        // Front tip
        points[0].x = static_cast<int>(boid.x + cosA * size);
        points[0].y = static_cast<int>(boid.y + sinA * size);

        // Back left
        points[1].x = static_cast<int>(boid.x - sinA * halfSize - cosA * halfSize);
        points[1].y = static_cast<int>(boid.y + cosA * halfSize - sinA * halfSize);

        // Back right
        points[2].x = static_cast<int>(boid.x + sinA * halfSize - cosA * halfSize);
        points[2].y = static_cast<int>(boid.y - cosA * halfSize - sinA * halfSize);

        // Back to front tip
        points[3].x = static_cast<int>(boid.x + cosA * size);
        points[3].y = static_cast<int>(boid.y + sinA * size);

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        
        // Draw the triangle
        SDL_RenderDrawLines(renderer, points, 4);
    }
}

void update_number_of_boids() {
    if (numBoidsSliderValue > numBoids) {
        // Add new boids with random init
        int toAdd = numBoidsSliderValue - numBoids;
        for (int i = 0; i < toAdd; ++i) {
            Boid b;
            b.x = rand() % window_width;
            b.y = rand() % window_height;
            b.vx = (rand() % 200 - 100) / 100.0f;
            b.vy = (rand() % 200 - 100) / 100.0f;
            boids.push_back(b);
        }
    } else {
        // Shrink the vector
        boids.resize(numBoidsSliderValue);
    }
    numBoids = numBoidsSliderValue;
}

void render_fps(SDL_Renderer* renderer) {
    static int frameCount = 0;
    static Uint32 lastTime = SDL_GetTicks();
    static std::string fpsText = "FPS: 0";

    frameCount++;
    Uint32 currentTime = SDL_GetTicks();

    if (currentTime - lastTime >= 500) { // Update once per half second
        float fps = frameCount / ((currentTime - lastTime) / 1000.0f);
        fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
        lastTime = currentTime;
        frameCount = 0;
    }

    // Render in top right corner
    render_text(renderer, fpsText, {255, 255, 255, 255}, 
                {window_width - 150, 10, 140, 30});
}

void main_loop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            #ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
            #endif
            running = false;
            SDL_Quit();
            return;
        }
        else if (e.type == SDL_MOUSEMOTION) {
            mouseX = e.motion.x;
            mouseY = e.motion.y;
        }
        handle_slider_event(e, visualRangeSlider);
        handle_slider_event(e, minAvoidanceSlider);
        handle_slider_event(e, speedLimitSlider);
        handle_slider_event(e, centeringFactorSlider);
        handle_slider_event(e, avoidanceFactorSlider);
        handle_slider_event(e, matchingFactorSlider);
        handle_slider_event(e, simulationDelaySlider);
        handle_slider_event(e, numBoidsSlider);
        handle_toggle_event(e, followMouseToggle);
    }

    if (numBoids != numBoidsSliderValue) {
        update_number_of_boids();
    }

    SDL_SetRenderDrawColor(renderer, 122, 122, 122, 255);
    SDL_RenderClear(renderer);

    update_boids_fast();
    render_boids_as_triangles();

    render_slider(renderer, visualRangeSlider);
    render_slider(renderer, minAvoidanceSlider);
    render_slider(renderer, speedLimitSlider);
    render_slider(renderer, centeringFactorSlider);
    render_slider(renderer, avoidanceFactorSlider);
    render_slider(renderer, matchingFactorSlider);
    render_slider(renderer, simulationDelaySlider);
    render_slider(renderer, numBoidsSlider);
    render_toggle(renderer, followMouseToggle);
    render_fps(renderer);

    SDL_RenderPresent(renderer);

}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    // For font rendering
    TTF_Init();
    init_font("assets/DejaVuSans-Bold.ttf", 16);

    window = SDL_CreateWindow("Boids", 100, 100, window_width, window_height, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // Initialize boids
    init_boids();

    srand(time(NULL));
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
    #else
    while (running) main_loop();
    #endif

    // cleanup
    cleanup_font();
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
