#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "game.h"

using namespace std;

static const char* kFontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
static constexpr int kWindowWidth = 1000;
static constexpr int kWindowHeight = 800;

int error(const string &s){
    cerr << s << SDL_GetError() << endl;
    return -1;
}

int init(SDL_Window* &window,SDL_Renderer* &renderer,int width,int height){
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return error("SDL could not initialize! SDL_Error: ");
    if (TTF_Init() == -1) return error("SDL_ttf could not initialize! SDL_Error: ");
    window = SDL_CreateWindow("Music Guessing Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window == NULL) return error("Window could not be created! SDL_Error:");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return error("Renderer could not be created! SDL_Error:");
    return 0;
}

void cleanup(SDL_Window* &window,SDL_Renderer* &renderer){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

int screen(int argc, char* args[])
{
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    int retval = init(window, renderer, kWindowWidth, kWindowHeight);
    if (retval != 0) return retval;

    TTF_Font* font = TTF_OpenFont(kFontPath, 28);
    if (font == NULL) {
        cleanup(window, renderer);
        cerr << "Failed to load font: " << kFontPath << " " << TTF_GetError() << endl;
        return -1;
    }

    retval = runGame(renderer, font, kWindowWidth, kWindowHeight);
    TTF_CloseFont(font);
    cleanup(window, renderer);
    
    return retval;
}
