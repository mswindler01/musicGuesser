#pragma once

#include <map>
#include <string>
#include <vector>

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
        std::string genreKey(const std::string& genre);
        std::string genreFilePath(const std::string& genre);
        std::string answerFilePath(const std::string& mode, const std::string& genre);
        bool loadGenreSections(
            const std::string& genre,
            std::map<std::string, std::vector<std::pair<std::string, std::string>>>& sections
        );
        bool loadEntriesFromFile(
            const std::string& filePath,
            std::vector<std::pair<std::string, std::string>>& entries,
            bool requireNumbering
        );
        size_t& modeIndex(const std::string& mode);
        std::string musicFilePath(const std::string& mode, const std::string& genre, const std::string& fileName);

    public:
        Level(std::string level = "");
        ~Level();
        std::string getLevel();
        bool isRadioOn() const;

        int lyrics(const std::string& genre, std::string& prompt, std::string& answer);
        int playMusic(const std::string& mode, const std::string& genre, std::string& prompt, std::string& answer);
        void stopAudio();
};
