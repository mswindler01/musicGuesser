
#include <iostream>
#include <string>
#include <SDL.h>
#include <vector>
#include <map>

using namespace std;

int error(string s){
    cerr << s << SDL_GetError()<<endl;
    return -1;
}

#include "MediaManager.hpp"
#include "Character.hpp"
#include "Game.hpp"
#include "MyGame.hpp"

MediaManager mm;

int main(int argc, char* args[]) {
    cout <<"Karl was here"<<endl;
    MyGame m;
    m.run();
    return 0;
}
