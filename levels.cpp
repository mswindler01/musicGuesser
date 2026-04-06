#define MINIAUDIO_IMPLEMENTATION
#include "levels.h"

#include <cctype>
#include <fstream>
#include <utility>
#include <vector>

using namespace std;

Level::Level(string level)
{
    _level = level;
    _lyricIndex = 0;
    _rhythmIndex = 0;
    _engineInitialized = false;
    _radioOn = false;
}

Level::~Level()
{
    stopAudio();
    if (_engineInitialized) {
        ma_engine_uninit(&_engine);
        _engineInitialized = false;
    }
}

string Level::getLevel()
{
    return _level;
}

bool Level::isRadioOn() const
{
    return _radioOn;
}

string Level::trim(string s)
{
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }

    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }

    return s.substr(start, end - start);
}

bool Level::Digits(string s)
{
    if (s.empty()) return false;
    for (char ch : s) {
        if (!isdigit(static_cast<unsigned char>(ch))) return false;
    }
    return true;
}

int Level::lyrics(string& prompt, string& answer)
{
    ifstream lyricsFile("lyrics.txt");
    vector<pair<string, string>> lyricsAndAnswers;
    string line;

    if (lyricsFile.is_open()) {
        while (getline(lyricsFile, line)) {
            string entry = trim(line);
            if (entry.empty()) continue;

            size_t dotPos = entry.find('.');
            if (dotPos == string::npos) continue;

            string idText = trim(entry.substr(0, dotPos));
            string content = trim(entry.substr(dotPos + 1));
            if (!Digits(idText) || content.empty()) continue;

            size_t semicolonPos = content.find(';');
            if (semicolonPos == string::npos) continue;

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
            prompt = "Error: lyrics.txt has no valid lyric entries";
            return 1;
        }

        const size_t index = _lyricIndex % lyricsAndAnswers.size();
        _lyricIndex++;
        prompt = "Lyric: " + lyricsAndAnswers[index].first;
        answer = lyricsAndAnswers[index].second;
        return 0;
    }

    prompt = "Error: lyrics.txt could not be opened";
    return 1;
}

void Level::melody()
{
}

int Level::rhythm(string& prompt, string& answer)
{
    ifstream rhythmFile("rhythm.txt");
    vector<pair<string, string>> songs;
    string line;

    if (!rhythmFile.is_open()) {
        prompt = "Error: rhythm.txt could not be opened";
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
        prompt = "Error: rhythm.txt has no valid song entries";
        return 1;
    }

    const size_t index = _rhythmIndex % songs.size();
    ++_rhythmIndex;
    const string fileName = songs[index].first;
    answer = songs[index].second;

    stopAudio();

    if (!_engineInitialized && ma_engine_init(NULL, &_engine) != MA_SUCCESS) {
        prompt = "Failed to init audio engine";
        return 1;
    }
    _engineInitialized = true;

    if (ma_engine_play_sound(&_engine, fileName.c_str(), NULL) != MA_SUCCESS) {
        _radioOn = false;
        prompt = "Failed to play " + fileName;
        return 1;
    }

    _radioOn = true;
    prompt = "Playing rhythm clip. Enter song title:";
    return 0;
}

void Level::stopAudio()
{
    _radioOn = false;
    if (_engineInitialized) {
        ma_engine_stop(&_engine);
    }
}