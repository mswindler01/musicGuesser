#pragma once

#include <sstream>
#include <string>


using namespace std;

class Sprite{
  protected:
    vector<SDL_Texture*> textures;
//  vector<SDL_Rect> srcs;
    SDL_Rect src,dst;
    int which;
    bool animated,dead;
    int totalMillis =0 ;
  void moveTo(int newX,int newY){
    dst.x=newX; dst.y=newY;
  }
  public:
  SDL_Rect getDestination() { return dst; }
  void stop(){ animated=false;}
  void go() {  animated=true;}
  void die(){ dead=true; }
  bool inside(int x,int y){
    return (dst.x<=x && x<=dst.x+dst.w && dst.y<=y && y<=dst.y+dst.h); 
  }
  bool collided(Sprite *other){
    return inside(other->dst.x,other->dst.y) ||
           inside(other->dst.x+other->dst.w,other->dst.y) ||
           inside(other->dst.x,other->dst.y+other->dst.h) ||
           inside(other->dst.x+other->dst.w,other->dst.y+other->dst.h);
  }
  void update(float dt){
    if (!animated || dead) return;
    totalMillis+=dt*1000; // add the dt amount of milliseconds
    int frameTime=250;
    while (totalMillis>frameTime) {
      which++;
      if (which>=textures.size()) which=0;
      totalMillis-=frameTime;
    }
  }
  void draw(SDL_Renderer *renderer,int offsetx=0,int offsety=0){
    if (dead) return;
//    if (which<0 || which>=textures.size()) throw "Invalid which";
    if (textures[which]==NULL) throw "About to render a null texture";
    SDL_Rect finalDst=dst;
    finalDst.x+=offsetx;
    finalDst.y+=offsety;
    cout << "Destination:" <<finalDst.x << ',' << finalDst.y << endl; 
    SDL_RenderCopy(renderer, textures[which], &src, &finalDst);
  }
  Sprite(SDL_Renderer *renderer,int count=1,string fname="image",string exten=".bmp",
      int newX=0,int newY=0){
    for (int i=0;i<count;i++){
      stringstream ss;
      ss << fname << i << exten;
      cout << ss.str() << endl;
      textures.push_back(mm.read(renderer,ss.str().c_str(),src.w,src.h));
    }
    src.x=0; src.y=0;
    dst.w=src.w; dst.h=src.h;
    moveTo(newX,newY);
    which=0;
    totalMillis;
    animated=true;
    dead=false;
  }
};

class Character:public Sprite{
    float px,py,vx,vy,ax,ay;
  public:
  Character(SDL_Renderer *renderer,int count=1,string fname="image",string exten=".bmp",
      int newX=0,int newY=0,
      float newVx=0.0,float newVy=0.0,
      float newAx=0.0,float newAy=0.0)
    :Sprite(renderer,count,fname,exten,newX,newY){
    //px=rand()%640-32.0;
    //py=rand()%240-16.0;
    //if (newX==0 && newY==0) {
      px=newX;
      py=newY;
      vx=newVx;
      vy=newVy;
      ax=newAx;
      ay=newAy;
    //}
  }
 
  void update(float dt){
    if (dead) return;
    Sprite::update(dt);
    vx=vx+ax*dt;
    px=px+vx*dt;
    vy=vy+ay*dt;
    py=py+vy*dt;
/*    if (px<0) px=640-32;
    if (px>640) px=0;
    if (py<0) py=480-32;
    if (py>480) py=0;*/
    moveTo(px,py);     
  }
  void keyEvent(SDL_Keycode symbol){
    if (symbol==SDLK_SPACE) {
            vx=vx+10.0;
            vy=vy+-10.0;
        }
    if (symbol==SDLK_a) px-=32.0;
    if (symbol==SDLK_d) px+=32.0;
    if (symbol==SDLK_w) py-=32.0;
    if (symbol==SDLK_s) py+=32.0;
  } 

};
