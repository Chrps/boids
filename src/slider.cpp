#include "slider.hpp"
#include "font.hpp"

float get_slider_position(const Slider& slider) {
    float normalized = (slider.get_value() - slider.minValue) / (slider.maxValue - slider.minValue);
    return slider.x + normalized * slider.w;
}

void render_slider(SDL_Renderer* renderer, Slider& slider) {
    SDL_Color textColor = {255, 255, 255, 255};
    if (slider.label) {
        SDL_Surface* labelSurface = TTF_RenderText_Solid(get_font(), slider.label, textColor);
        SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(renderer, labelSurface);
        SDL_Rect labelRect = {
            slider.x,
            slider.y - labelSurface->h - 4,
            labelSurface->w,
            labelSurface->h
        };
        SDL_RenderCopy(renderer, labelTexture, NULL, &labelRect);
        SDL_FreeSurface(labelSurface);
        SDL_DestroyTexture(labelTexture);
    }
    // Draw slider track
    SDL_Rect track = {slider.x, slider.y + slider.h / 2 - 2, slider.w, 4};
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &track);

    // Calculate knob position from current value
    float knobX = get_slider_position(slider);
    SDL_Rect knob = { static_cast<int>(knobX) - 5, slider.y, 10, slider.h };

    // Draw knob
    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
    SDL_RenderFillRect(renderer, &knob);

    // Convert value to string
    char buf[16];
    if (slider.type == SliderType::INT)
        snprintf(buf, sizeof(buf), "%d", static_cast<int>(slider.get_value()));
    else
        snprintf(buf, sizeof(buf), "%.4f", slider.get_value());

    // Create surface and texture for text
    SDL_Surface* textSurface = TTF_RenderText_Solid(get_font(), buf, textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Position text next to the slider
    SDL_Rect textRect = {slider.x + slider.w + 10, slider.y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void handle_slider_event(const SDL_Event& e, Slider& slider) {
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    if (e.type == SDL_MOUSEBUTTONDOWN) {
        float t = (slider.get_value() - slider.minValue) / (slider.maxValue - slider.minValue);
        int handleX = slider.x + int(t * slider.w);
        SDL_Rect handleRect = { handleX - 5, slider.y - 5, 10, slider.h + 10 };

        if (mx >= handleRect.x && mx <= handleRect.x + handleRect.w &&
            my >= handleRect.y && my <= handleRect.y + handleRect.h) {
            slider.dragging = true;
        }
    }
    else if (e.type == SDL_MOUSEBUTTONUP) {
        slider.dragging = false;
    }
    else if (e.type == SDL_MOUSEMOTION && slider.dragging) {
        float t = (float)(mx - slider.x) / (float)slider.w;
        if (t < 0) t = 0;
        if (t > 1) t = 1;
        slider.set_value(slider.minValue + t * (slider.maxValue - slider.minValue));
    }
}