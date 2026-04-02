#pragma once

#include <string>

#include "miniaudio.h"

class Level
{
    private:
        std::string _level;
        size_t _lyricIndex;
        size_t _rhythmIndex;
        ma_engine _engine;
        bool _engineInitialized;

    protected:
        bool Digits(std::string s);
        std::string trim(std::string s);

    public:
        Level(std::string level = "");
        ~Level();
        std::string getLevel();

        int lyrics(std::string& prompt, std::string& answer);
        void melody();
        int rhythm(std::string& prompt, std::string& answer);
        void stopAudio();
};
