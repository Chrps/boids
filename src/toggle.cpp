#include "toggle.hpp"
#include "font.hpp"

void render_toggle(SDL_Renderer* renderer, const Toggle& toggle) {
    SDL_Color textColor = {255, 255, 255, 255};
    
    // Draw label above the box
    if (toggle.label) {
        SDL_Surface* labelSurface = TTF_RenderText_Solid(get_font(), toggle.label, textColor);
        SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(renderer, labelSurface);
        SDL_Rect labelRect = { toggle.x, toggle.y - labelSurface->h - 4, labelSurface->w, labelSurface->h };
        SDL_RenderCopy(renderer, labelTexture, NULL, &labelRect);
        SDL_FreeSurface(labelSurface);
        SDL_DestroyTexture(labelTexture);
    }

    // Box color based on toggle state
    SDL_SetRenderDrawColor(renderer, *toggle.value ? 0 : 200, *toggle.value ? 180 : 50, 0, 255);
    SDL_Rect box = { toggle.x, toggle.y, toggle.w, toggle.h };
    SDL_RenderFillRect(renderer, &box);

    // Text inside box
    const char* text = *toggle.value ? "On" : "Off";
    SDL_Surface* textSurface = TTF_RenderText_Solid(get_font(), text, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Center text in box
    SDL_Rect textRect = {
        toggle.x + (toggle.w - textSurface->w) / 2,
        toggle.y + (toggle.h - textSurface->h) / 2,
        textSurface->w,
        textSurface->h
    };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void handle_toggle_event(const SDL_Event& e, const Toggle& toggle) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mx = e.button.x;
        int my = e.button.y;

        SDL_Rect box = { toggle.x, toggle.y, toggle.w, toggle.h };
        if (mx >= box.x && mx <= box.x + box.w && my >= box.y && my <= box.y + box.h) {
            *toggle.value = !(*toggle.value);
        }
    }
}
