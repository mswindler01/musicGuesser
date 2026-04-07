#pragma once

class MediaManager{
    map<string,SDL_Texture *> images;
    public:
    SDL_Texture* read(SDL_Renderer *renderer,string fname,int &w,int &h){
        if (images.find(fname)==images.end()){
          string fullPath="./resources/images/"+fname;
          SDL_Surface* character=SDL_LoadBMP(fullPath.c_str());
          if (character==NULL) throw "Could not read image.bmp file";
          SDL_SetColorKey(character,SDL_TRUE,SDL_MapRGB(character->format,173,54,58));
          SDL_Texture* charText=SDL_CreateTextureFromSurface(renderer,character);
          SDL_FreeSurface(character);
          if(charText==NULL) throw "Failed to create texture";
          images[fname]=charText;
        }
        SDL_Texture *result=images[fname];
        SDL_QueryTexture(result,NULL,NULL,&w,&h);
        return result;
    }
    ~MediaManager() {
        for (auto entry:images) SDL_DestroyTexture(entry.second);
    }
};
extern MediaManager mm;
