#include "text.hpp"
#include "font.hpp"

void render_text(SDL_Renderer* renderer, const std::string& text,
                 SDL_Color color, const SDL_Rect& container) {
    TTF_Font* font = get_font();
    if (!font) return;

    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dstRect = {
        container.x + (container.w - surface->w) / 2,
        container.y + (container.h - surface->h) / 2,
        surface->w,
        surface->h
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}