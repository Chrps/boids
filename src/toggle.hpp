#pragma once

#include <SDL.h>

struct Toggle {
    int x, y, w, h;
    bool* value;
    const char* label;
};

void render_toggle(SDL_Renderer* renderer, const Toggle& toggle);
void handle_toggle_event(const SDL_Event& e, const Toggle& toggle);
