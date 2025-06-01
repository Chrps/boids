#include "font.hpp"
#include <SDL_log.h>

static TTF_Font* font = nullptr;

bool init_font(const char* path, int size) {
    if (TTF_Init() == -1) {
        SDL_Log("TTF_Init error: %s", TTF_GetError());
        return false;
    }
    font = TTF_OpenFont(path, size);
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
        return false;
    }
    return true;
}

void cleanup_font() {
    if (font) TTF_CloseFont(font);
    TTF_Quit();
}

TTF_Font* get_font() {
    return font;
}