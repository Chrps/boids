#ifndef FONT_HPP
#define FONT_HPP

#include <SDL_ttf.h>

bool init_font(const char* path = "assets/DejaVuSans-Bold.ttf", int size = 16);
void cleanup_font();
TTF_Font* get_font();

#endif