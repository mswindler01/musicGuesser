#include <iostream>
#include <SDL.h>
#include <string>

using namespace std;

int error(const string &s){
    cerr << s << SDL_GetError() << endl;
    return -1;
}

int init(SDL_Window* &window,SDL_Renderer* &renderer,int width,int height){
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return error("SDL could not initialize! SDL_Error: ");
    window = SDL_CreateWindow("Music Guessing Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (window == NULL) return error("Window could not be created! SDL_Error:");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return error("Renderer could not be created! SDL_Error:");
    return 0;
}

void cleanup(SDL_Window* &window,SDL_Renderer* &renderer){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int screen(int argc, char* args[])
{
    cout << "Arg 0 " << args[0]<< endl;
    cout << "Arg 1 " << args[1]<< endl;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    int retval=init(window,renderer,1000,800);
    if (retval!=0) return retval;
    
    bool quit = false;

    SDL_Event e;

    while (!quit) {

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT)  quit = true;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(1000/240);
        
    }
    cleanup(window,renderer);
    
    return 0;
}
