#pragma once

#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

using namespace std;

class Level
{
    private:
        string _level;
        size_t _lyricIndex;
        size_t _rhythmIndex;

    protected:
        bool Digits(string s);
        string trim(string s);

    public:
        Level(string = "");
        string getLevel();

        int lyrics(string& answer);
        void melody();
        int rhythm(string& answer);
};

inline Level::Level(string level)
{
    _level = level;
    _lyricIndex = 0;
    _rhythmIndex = 0;
}

inline string Level::getLevel()
{
    return _level;
}

inline string Level::trim(string s)
{
    size_t start = 0;
    while (start < s.size() && isspace(s[start])) {
        start++;
    }

    size_t end = s.size();
    while (end > start && isspace(s[end - 1])) {
        end--
        ;
    }

    return s.substr(start, end - start);
}

inline bool Level::Digits(string s)
{
    if (s.empty()) return false;
    for (char ch : s) {
        if (!isdigit(ch)) return false;
    }
    return true;
}

inline int Level::lyrics(string& answer)
{
    ifstream lyricsFile("lyrics.txt");
    vector<pair<string, string>> lyricsAndAnswers;
    string line;

    if (lyricsFile.is_open()) {
        while (getline(lyricsFile, line)) {
            // Remove leading/trailing spaces 
            string entry = trim(line);
            if (entry.empty()) continue;

            // Find the beginning number
            size_t dotPos = entry.find('.');
            if (dotPos == string::npos) continue;

            // Split line into id text and everything after the dot
            string idText = trim(entry.substr(0, dotPos));
            string content = trim(entry.substr(dotPos + 1));
            if (!Digits(idText) || content.empty()) continue;

            // Find separator between lyric and answer
            size_t semicolonPos = content.find(';');
            if (semicolonPos == string::npos) continue;

            // Left side is lyric text, right side is song title answer
            string lyricText = trim(content.substr(0, semicolonPos));
            string answerText = trim(content.substr(semicolonPos + 1));
            if (lyricText.empty() || answerText.empty()) continue;

            if (lyricText.size() >= 2 && lyricText.front() == '"' && lyricText.back() == '"') {
                lyricText = lyricText.substr(1, lyricText.size() - 2);
            }

            lyricsAndAnswers.push_back({lyricText, answerText});
        }
        lyricsFile.close();

        if (lyricsAndAnswers.empty()) {
            cout << "Error: lyrics.txt has no valid lyric entries" << endl;
            return 1;
        }

        const size_t index = _lyricIndex % lyricsAndAnswers.size();
        ++_lyricIndex;
        cout << "Lyric: " << lyricsAndAnswers[index].first << endl;
        answer = lyricsAndAnswers[index].second;
        return 0;
    }
    else {
        cout << "Error: lyrics.txt could not be opened" << endl;
        return 1;
    }
}

inline void Level::melody()
{
}

inline int Level::rhythm(string& answer)
{
    ifstream rhythmFile("rhythm.txt");
    // Hold valid entries as (audio file, answer title)
    vector<pair<string, string>> songs;
    string line;

    if (!rhythmFile.is_open()) {
        cout << "Error: rhythm.txt could not be opened" << endl;
        return 1;
    }

    while (getline(rhythmFile, line)) {
        string entry = trim(line);
        if (entry.empty()) continue;

        size_t dotPos = entry.find('.');
        string content = entry;
        if (dotPos != string::npos) {
            string idText = trim(entry.substr(0, dotPos));
            if (!Digits(idText)) {
                continue;
            }
            content = trim(entry.substr(dotPos + 1));
        }

        // Split "audio file ; answer title"
        size_t semicolonPos = content.find(';');
        if (semicolonPos == string::npos) {
            continue;
        }

        string fileText = trim(content.substr(0, semicolonPos));
        string answerText = trim(content.substr(semicolonPos + 1));
        if (fileText.empty() || answerText.empty()) {
            continue;
        }

        songs.push_back({fileText, answerText});
    }
    rhythmFile.close();

    if (songs.empty()) {
        cout << "Error: rhythm.txt has no valid song entries" << endl;
        return 1;
    }

    // Cycle songs in file order, one per round.
    const size_t index = _rhythmIndex % songs.size();
    ++_rhythmIndex;
    const string fileName = songs[index].first;
    answer = songs[index].second;

    // Initialize miniaudio engine.
    ma_engine engine;
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        cerr << "Failed to init audio engine\n";
        return 1;
    }

    // Play the selected rhythm clip.
    if (ma_engine_play_sound(&engine, fileName.c_str(), NULL) != MA_SUCCESS) {
        cerr << "Failed to play " << fileName << "\n";
        ma_engine_uninit(&engine);
        return 1;
    }

    // Clear leftover input, then wait for Enter to stop playback
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Playing... press Enter to quit\n";
    cin.get();

    // Clean up audio engine resources.
    ma_engine_uninit(&engine);
    return 0;
}
