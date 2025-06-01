#ifndef SLIDER_HPP
#define SLIDER_HPP

#include <SDL.h>

enum class SliderType { INT, FLOAT };

struct Slider {
    int x, y, w, h;
    SliderType type;
    union {
        int* intValue;
        float* floatValue;
    };
    float minValue, maxValue;
    const char* label;
    bool dragging = false;

    // Helper functions to get/set value as float internally
    float get_value() const {
        if (type == SliderType::INT) return static_cast<float>(*intValue);
        else return *floatValue;
    }
    void set_value(float v) {
        if (type == SliderType::INT) {
            int iv = static_cast<int>(v + 0.5f); // round to int
            if (*intValue != iv) *intValue = iv;
        }
        else {
            *floatValue = v;
        }
    }
};

void handle_slider_event(const SDL_Event& e, Slider& slider);
void render_slider(SDL_Renderer* renderer, Slider& slider);

#endif