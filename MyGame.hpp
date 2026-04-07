#pragma once

const int WIDTH=640;
const int HEIGHT=480;

class MyGame:public Game{
    // That characters and player are in game space not screen space
    vector<Character*> characters;
    Character *player; // Player x and y are the center of the screen we render
    int rcount,bcount;
    void setup() {
      //  cout << "Creating Square" << endl;
        for (int i=-100;i<100;i++)
          for (int j=-100;j<100;j++)
              characters.push_back(new Character(renderer,4,"image",".bmp",i*32,j*32));
//        characters.push_back(new Character(renderer));
        player=new Character(renderer,1,"karl0",".bmp",0,0);
    }
    void update(float dt){
        for (auto c:characters)  c->update(dt);
        for (auto c:characters) if (player->collided(c)) c->die();
        player->update(dt);
    }
    void keyHandler(SDL_Keycode symbol){
        player->keyEvent(symbol);
    }
    void draw(){
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Gray color
        SDL_Rect d=player->getDestination();
        for (auto c:characters) c->draw(renderer,(WIDTH-d.w)/2-d.x,(HEIGHT-d.h)/2-d.y);
        player->draw(renderer,WIDTH/2-d.x-d.w/2,HEIGHT/2-d.y-d.h/2);
    }

    public:
    MyGame():Game(WIDTH,HEIGHT,"Our  Game"){ 
        rcount=0;
        bcount=0;
    }
};

