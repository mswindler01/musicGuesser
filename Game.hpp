#pragma once

class Game{
    SDL_Window* window;
    bool quit;
    
    int init(int width,int height,string windowTitle){
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) throw "SDL could not initialize! SDL_Error: "; 
        window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
        if (window == NULL) throw "Window could not be created! SDL_Error:";
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL) throw "Renderer could not be created! SDL_Error:";
        return 0;
    }

    protected:
    /* Your game must implement these 4 functions to work properly */
    virtual void setup()=0;
    virtual void update(float dt)=0;
    virtual void keyHandler(SDL_Keycode symbol)=0;
    virtual void draw()=0;
    SDL_Renderer* renderer;
    string windowTitle;
    int width,height;
    void gameOver(){quit=true;}
    
    public:
    Game(int newWidth=640,int newHeight=480,string newTitle="Boring Title"){ 
        width=newWidth;
        height=newHeight;
        windowTitle=newTitle; 
    }
    void run(){
        window = NULL;
        renderer = NULL;
        quit = false;
        SDL_Event e;
        try {
            int retval=init(640,480,windowTitle);
            setup();
            int last=SDL_GetTicks();
            while (!quit) {
                // double integrate accelerations to update velocity->position
                int now=SDL_GetTicks();
                float dt=(float)(now-last)/1000.0;
                last=now;
                update(dt);
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT)  quit = true;
                    else if (e.type== SDL_KEYDOWN){
                        if (e.key.type== SDL_KEYDOWN) keyHandler(e.key.keysym.sym);
                    }
                }
                SDL_RenderClear(renderer);
                draw();
                SDL_RenderPresent(renderer);
                SDL_Delay(1000/240);
            }
           // cleanup(window,renderer);
        }
        catch (char const* err) {
            cerr << err << endl; 
        }
    }
    ~Game(){
      SDL_DestroyRenderer(renderer);
      SDL_DestroyWindow(window);
      SDL_Quit();
    }
};

