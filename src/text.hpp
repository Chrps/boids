#pragma once
#include <SDL.h>
#include <string>

void render_text(SDL_Renderer* renderer, const std::string& text,
                 SDL_Color color, const SDL_Rect& container);
