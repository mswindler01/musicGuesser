#pragma once

#include <string>

#include "miniaudio.h"

class Level
{
    private:
        std::string _level;
        size_t _lyricIndex;
        size_t _rhythmIndex;
        size_t _melodyIndex;
        ma_engine _engine;
        ma_sound _sound;
        bool _soundInitialized;
        bool _engineInitialized;
        bool _radioOn;

    protected:
        bool Digits(std::string s);
        std::string trim(std::string s);

    public:
        Level(std::string level = "");
        ~Level();
        std::string getLevel();
        bool isRadioOn() const;

        int lyrics(std::string& prompt, std::string& answer);
        int melody(std::string& prompt, std::string& answer);
        int rhythm(std::string& prompt, std::string& answer);
        void stopAudio();
};