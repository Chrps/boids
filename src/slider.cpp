#include "slider.hpp"
#include "font.hpp"

float get_slider_position(const Slider& slider) {
    float normalized = (slider.get_value() - slider.minValue) / (slider.maxValue - slider.minValue);
    return slider.x + normalized * slider.w;
}

void render_slider(SDL_Renderer* renderer, Slider& slider) {
    SDL_Color textColor = {255, 255, 255, 255};

    // Render label (cached texture)
    if (slider.label && !slider.labelTexture) {
        SDL_Surface* labelSurface = TTF_RenderText_Solid(get_font(), slider.label, textColor);
        slider.labelTexture = SDL_CreateTextureFromSurface(renderer, labelSurface);
        slider.labelW = labelSurface->w;
        slider.labelH = labelSurface->h;
        SDL_FreeSurface(labelSurface);
    }
    if (slider.labelTexture) {
        SDL_Rect labelRect = {
            slider.x,
            slider.y - slider.labelH - 4,
            slider.labelW,
            slider.labelH
        };
        SDL_RenderCopy(renderer, slider.labelTexture, NULL, &labelRect);
    }

    // Draw slider track
    SDL_Rect track = {slider.x, slider.y + slider.h / 2 - 2, slider.w, 4};
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderFillRect(renderer, &track);

    // Calculate knob position
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

    // Use your render_text function for value text (no caching)
    int textW = 0, textH = 0;
    TTF_SizeText(get_font(), buf, &textW, &textH);

    SDL_Rect valueRect = {
        slider.x + slider.w + 10,      // base position
        slider.y + (slider.h - textH)/2,  // vertically centered
        textW,
        textH
    };
    render_text(renderer, buf, textColor, valueRect);
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