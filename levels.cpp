#define MINIAUDIO_IMPLEMENTATION
#include "levels.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include <map>
#include <utility>
#include <vector>

using namespace std;

namespace {
const char* kAnswersDir = "resources/answers/";
const char* kMusicDir = "resources/music/";
}

Level::Level(string level)
{
    _level = level;
    _lyricIndex = 0;
    _rhythmIndex = 0;
    _melodyIndex =0;
    _engineInitialized = false;
    _soundInitialized = false;
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

string Level::genreKey(const string& genre)
{
    string key;
    for (char ch : genre) {
        if (isalnum(static_cast<unsigned char>(ch))) {
            key += static_cast<char>(tolower(static_cast<unsigned char>(ch)));
        } else if (isspace(static_cast<unsigned char>(ch)) && !key.empty() && key.back() != '_') {
            key += '_';
        }
    }

    while (!key.empty() && key.back() == '_') {
        key.pop_back();
    }

    return key;
}

string Level::genreFilePath(const string& genre)
{
    const string key = genreKey(genre);
    if (key.empty()) {
        return "";
    }

    return string(kAnswersDir) + key + ".txt";
}

string Level::answerFilePath(const string& mode, const string& genre)
{
    const string key = genreKey(genre);
    if (!key.empty()) {
        const string genreFile = string(kAnswersDir) + key + "_" + mode + ".txt";
        if (filesystem::exists(genreFile)) {
            return genreFile;
        }
    }

    return string(kAnswersDir) + mode + ".txt";
}

bool Level::loadEntriesFromFile(const string& filePath, vector<pair<string, string>>& entries, bool requireNumbering)
{
    ifstream file(filePath);
    string line;

    if (!file.is_open()) {
        return false;
    }

    while (getline(file, line)) {
        string entry = trim(line);
        if (entry.empty()) {
            continue;
        }

        size_t dotPos = entry.find('.');
        string content = entry;

        if (dotPos != string::npos) {
            string idText = trim(entry.substr(0, dotPos));
            if (Digits(idText)) {
                content = trim(entry.substr(dotPos + 1));
            } else if (requireNumbering) {
                continue;
            }
        } else if (requireNumbering) {
            continue;
        }

        size_t semicolonPos = content.find(';');
        if (semicolonPos == string::npos) {
            continue;
        }

        string promptText = trim(content.substr(0, semicolonPos));
        string answerText = trim(content.substr(semicolonPos + 1));
        if (promptText.empty() || answerText.empty()) {
            continue;
        }

        if (promptText.size() >= 2 && promptText.front() == '"' && promptText.back() == '"') {
            promptText = promptText.substr(1, promptText.size() - 2);
        }

        entries.push_back({promptText, answerText});
    }

    return true;
}

bool Level::loadGenreSections(const string& genre, map<string, vector<pair<string, string>>>& sections)
{
    const string filePath = genreFilePath(genre);
    if (filePath.empty() || !filesystem::exists(filePath)) {
        return false;
    }

    ifstream file(filePath);
    string line;
    string currentSection;

    if (!file.is_open()) {
        return false;
    }

    while (getline(file, line)) {
        string entry = trim(line);
        if (entry.empty()) {
            continue;
        }

        if (entry.front() == '[' && entry.back() == ']') {
            currentSection = trim(entry.substr(1, entry.size() - 2));
            transform(currentSection.begin(), currentSection.end(), currentSection.begin(), [](unsigned char ch) {
                return static_cast<char>(tolower(ch));
            });
            continue;
        }

        if (currentSection.empty()) {
            continue;
        }

        size_t semicolonPos = entry.find(';');
        if (semicolonPos == string::npos) {
            continue;
        }

        string promptText = trim(entry.substr(0, semicolonPos));
        string answerText = trim(entry.substr(semicolonPos + 1));
        if (promptText.empty() || answerText.empty()) {
            continue;
        }

        if (promptText.size() >= 2 && promptText.front() == '"' && promptText.back() == '"') {
            promptText = promptText.substr(1, promptText.size() - 2);
        }

        sections[currentSection].push_back({promptText, answerText});
    }

    return true;
}

bool Level::Digits(string s)
{
    if (s.empty()) return false;
    for (char ch : s) {
        if (!isdigit(static_cast<unsigned char>(ch))) return false;
    }
    return true;
}

size_t& Level::modeIndex(const string& mode)
{
    if (mode == "rhythm") {
        return _rhythmIndex;
    }
    return _melodyIndex;
}

string Level::musicFilePath(const string& mode, const string& genre, const string& fileName)
{
    const filesystem::path musicRoot(kMusicDir);
    const filesystem::path requestedPath(fileName);

    if (requestedPath.is_absolute() && filesystem::exists(requestedPath)) {
        return requestedPath.string();
    }

    const vector<filesystem::path> candidates = {
        musicRoot / fileName,
        musicRoot / mode / fileName,
        musicRoot / mode / genre / fileName,
        musicRoot / mode / genreKey(genre) / fileName
    };

    for (const filesystem::path& candidate : candidates) {
        if (filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    const filesystem::path modePath = musicRoot / mode;
    if (filesystem::exists(modePath)) {
        for (const auto& entry : filesystem::recursive_directory_iterator(modePath)) {
            if (entry.is_regular_file() && entry.path().filename() == requestedPath.filename()) {
                return entry.path().string();
            }
        }
    }

    return (musicRoot / mode / fileName).string();
}

int Level::lyrics(const string& genre, string& prompt, string& answer)
{
    map<string, vector<pair<string, string>>> sections;
    if (loadGenreSections(genre, sections) && !sections["lyrics"].empty()) {
        const vector<pair<string, string>>& lyricsAndAnswers = sections["lyrics"];
        const size_t index = _lyricIndex % lyricsAndAnswers.size();
        _lyricIndex++;
        prompt = "Lyric: " + lyricsAndAnswers[index].first;
        answer = lyricsAndAnswers[index].second;
        return 0;
    }

    const string filePath = answerFilePath("lyrics", genre);
    vector<pair<string, string>> lyricsAndAnswers;

    if (!loadEntriesFromFile(filePath, lyricsAndAnswers, true)) {
        prompt = "Error: " + filePath + " could not be opened";
        return 1;
    }

    if (lyricsAndAnswers.empty()) {
        prompt = "Error: " + filePath + " has no valid lyric entries";
        return 1;
    }

    const size_t index = _lyricIndex % lyricsAndAnswers.size();
    _lyricIndex++;
    prompt = "Lyric: " + lyricsAndAnswers[index].first;
    answer = lyricsAndAnswers[index].second;
    return 0;
}

int Level::playMusic(const string& mode, const string& genre, string& prompt, string& answer)
{
    if (mode != "melody" && mode != "rhythm") {
        prompt = "Error: invalid music mode";
        return 1;
    }

    const string promptLabel = (mode == "melody") ? "melody" : "rhythm";
    size_t& currentIndex = modeIndex(mode);
    map<string, vector<pair<string, string>>> sections;
    if (loadGenreSections(genre, sections) && !sections[mode].empty()) {
        const vector<pair<string, string>>& songs = sections[mode];
        const size_t index = currentIndex % songs.size();
        ++currentIndex;

        const string fileName = musicFilePath(mode, genre, songs[index].first);
        answer = songs[index].second;

        stopAudio();

        if (!_engineInitialized && ma_engine_init(NULL, &_engine) != MA_SUCCESS) {
            prompt = "Failed to init audio engine";
            return 1;
        }
        _engineInitialized = true;

        if (ma_sound_init_from_file(&_engine, fileName.c_str(), 0, NULL, NULL, &_sound) != MA_SUCCESS) {
            _radioOn = false;
            prompt = "Failed to load " + fileName;
            return 1;
        }

        ma_sound_start(&_sound);
        _soundInitialized = true;
        _radioOn = true;
        prompt = "Playing " + promptLabel + " clip. Enter song title:";
        return 0;
    }

    const string filePath = answerFilePath(mode, genre);
    vector<pair<string, string>> songs;

    if (!loadEntriesFromFile(filePath, songs, false)) {
        prompt = "Error: " + filePath + " could not be open";
        return 1;
    }

    if (songs.empty()) {
        prompt = "Error: " + filePath + " has no valid entries";
        return 1;
    }

    const size_t index = currentIndex % songs.size();
    ++currentIndex;

    const string fileName = musicFilePath(mode, genre, songs[index].first);
    answer = songs[index].second;

    stopAudio();

    if (!_engineInitialized) {
        if (ma_engine_init(NULL, &_engine) != MA_SUCCESS) {
            prompt = "Failed to init audio engine";
            return 1;
        }
        _engineInitialized = true;
    }

    if (ma_sound_init_from_file(&_engine, fileName.c_str(), 0, NULL, NULL, &_sound) != MA_SUCCESS) {
        _radioOn = false;
        prompt = "Failed to load " + fileName;
        return 1;
    }

    ma_sound_start(&_sound);
    _soundInitialized = true;
    _radioOn = true;
    prompt = "Playing " + promptLabel + " clip. Enter song title:";
    return 0;
}

void Level::stopAudio()
{
    _radioOn = false;
    if (_soundInitialized) {
        ma_sound_stop(&_sound);
        ma_sound_uninit(&_sound);
        _soundInitialized = false;
    }

    if (_engineInitialized) {
        ma_engine_uninit(&_engine);
        _engineInitialized = false;
    }
}
