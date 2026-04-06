#pragma once

#include <SDL.h>
#include <string>

struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

int error(const std::string &s);

int init(SDL_Window* &window,SDL_Renderer* &renderer,int width,int height);

void cleanup(SDL_Window* &window,SDL_Renderer* &renderer);

int screen(int argc, char* args[]);