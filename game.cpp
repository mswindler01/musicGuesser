#include "game.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_surface.h>

#include "levels.h"

using namespace std;

static const int padding = 24;
static const size_t maxMessages = 14;

enum class AppState {
    ChooseMode,
    EnterGuess,
    AskReplay
};

struct GameData {
    Level level;
    vector<string> messages;
    string currentAnswer;
    string inputBuffer;
    AppState state;
    bool quit;
    string mode;
};

// Remove spaces from the front and back of the player's input.
static string trimCopy(const string& text)
{
    size_t start = 0;
    while (start < text.size() && isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }

    size_t end = text.size();
    while (end > start && isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }

    return text.substr(start, end - start);
}

// Convert text to lowercase so comparisons like "Lyrics" vs "lyrics" match.
static string lowerCopy(string text)
{
    transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    });
    return text;
}

// Keep a short on-screen history instead of letting messages grow forever.
static void pushMessage(vector<string>& messages, const string& message)
{
    if (message.empty()) {
        return;
    }

    messages.push_back(message);
    if (messages.size() > maxMessages) {
        messages.erase(messages.begin());
    }
}

static SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& fileName)
{
    SDL_Surface* surface = SDL_LoadBMP(fileName.c_str());
    if (surface == NULL) {
        return NULL;
    }

    Uint32 key = SDL_MapRGB(surface->format, 255, 255, 255);
    SDL_SetColorKey(surface, SDL_TRUE, key);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

static void destroyTextureArray(vector<SDL_Texture*>& textures)
{
    for (SDL_Texture* texture : textures) {
        if (texture != NULL) {
            SDL_DestroyTexture(texture);
        }
    }
    textures.clear();
}

static void cleanupVisuals(vector<SDL_Texture*>& backgroundTextures, vector<SDL_Texture*>& radioTextures, SDL_Texture* interiorTexture)
{
    destroyTextureArray(backgroundTextures);
    destroyTextureArray(radioTextures);
    if (interiorTexture != NULL) {
        SDL_DestroyTexture(interiorTexture);
    }
}

// Draw one block of wrapped text by turning it into a surface, then a texture.
static bool renderTextBlock(SDL_Renderer* renderer, TTF_Font* font, const string& text, SDL_Color color, int x, int y, int wrapWidth)
{
    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, wrapWidth);
    if (surface == NULL) {
        return false;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_FreeSurface(surface);
        return false;
    }

    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
    return true;
}

// Start a new round and ask Level for the prompt and answer.
static void beginRound(GameData& game, const string& nextMode)
{
    string prompt;
    game.currentAnswer.clear();
    game.mode = nextMode;

    if (game.mode == "lyrics") {
        if (game.level.lyrics(prompt, game.currentAnswer) != 0) {
            pushMessage(game.messages, prompt);
            game.quit = true;
            return;
        }
    } else if (game.mode == "rhythm") {
        if (game.level.rhythm(prompt, game.currentAnswer) != 0) {
            pushMessage(game.messages, prompt);
            game.quit = true;
            return;
        }
    }else if(game.mode == "melody"){
        if(game.level.melody(prompt, game.currentAnswer) != 0){
            pushMessage(game.messages, prompt);
            game.quit= true;
            return;
        }
    }

    pushMessage(game.messages, prompt);
    if (game.mode == "lyrics") {
        pushMessage(game.messages, "Enter song title:");
    }
    game.state = AppState::EnterGuess;
}

// Handle Enter based on the current screen the player is on.
static void submitInput(GameData& game, const string& rawInput)
{
    string value = trimCopy(rawInput);
    if (value.empty()) {
        return;
    }

    if (game.state == AppState::ChooseMode) {
        string normalized = lowerCopy(value);
        if (normalized == "lyrics" || normalized == "rhythm"|| normalized == "melody") {
            beginRound(game, normalized);
        } else {
            pushMessage(game.messages, "Invalid type");
            pushMessage(game.messages, "Would you like to guess by the lyrics, the rhythm or melody?");
        }
        return;
    }

    if (game.state == AppState::EnterGuess) {
        game.level.stopAudio();
        if (value == game.currentAnswer) {
            pushMessage(game.messages, "You guessed correctly!");
        } else {
            pushMessage(game.messages, "You guessed wrong");
        }
        pushMessage(game.messages, "Play again? (y/n):");
        game.state = AppState::AskReplay;
        return;
    }

    string normalized = lowerCopy(value);
    if (normalized == "y") {
        pushMessage(game.messages, "Would you like to guess by the lyrics, the rhythm or the melody?");
        game.state = AppState::ChooseMode;
    } else if (normalized == "n") {
        game.quit = true;
    } else {
        pushMessage(game.messages, "Play again? (y/n):");
    }
}

int runGame(SDL_Renderer* renderer, TTF_Font* font, int width, int height)
{
    GameData game;
    game.state = AppState::ChooseMode;
    game.quit = false;

    vector<SDL_Texture*> backgroundTextures;
    vector<SDL_Texture*> radioTextures;

    backgroundTextures.push_back(loadTexture(renderer, "resources/images/background0.bmp"));
    backgroundTextures.push_back(loadTexture(renderer, "resources/images/background1.bmp"));
    backgroundTextures.push_back(loadTexture(renderer, "resources/images/background2.bmp"));
    backgroundTextures.push_back(loadTexture(renderer, "resources/images/background3.bmp"));

    radioTextures.push_back(loadTexture(renderer, "resources/images/radio0.bmp"));
    radioTextures.push_back(loadTexture(renderer, "resources/images/radio1.bmp"));
    radioTextures.push_back(loadTexture(renderer, "resources/images/radio2.bmp"));
    radioTextures.push_back(loadTexture(renderer, "resources/images/radio3.bmp"));

    SDL_Texture* interiorTexture = loadTexture(renderer, "resources/images/interior0.bmp");

    for (SDL_Texture* texture : backgroundTextures) {
        if (texture == NULL) {
            cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
            return -1;
        }
    }

    for (SDL_Texture* texture : radioTextures) {
        if (texture == NULL) {
            cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
            return -1;
        }
    }

    if (interiorTexture == NULL) {
        cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
        return -1;
    }

    int backgroundFrame = 0;
    int radioFrame = 0;
    Uint32 lastBackgroundTick = SDL_GetTicks();
    Uint32 lastRadioTick = SDL_GetTicks();

    pushMessage(game.messages, "Hello! Welcome to my music guessing game.");
    pushMessage(game.messages, "Would you like to guess by the lyrics, the rhythm or the melody?");

    SDL_Event e;
    SDL_StartTextInput();

    // Main game loop: read input, update state, then draw the current screen.
    while (!game.quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                game.quit = true;
            } else if (e.type == SDL_TEXTINPUT) {
                game.inputBuffer += e.text.text;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !game.inputBuffer.empty()) {
                    game.inputBuffer.pop_back();
                } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    submitInput(game, game.inputBuffer);
                    game.inputBuffer.clear();
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    game.quit = true;
                }
            }
        }

        Uint32 currentTick = SDL_GetTicks();

        if (currentTick - lastBackgroundTick >= 120) {
            backgroundFrame++;
            if (backgroundFrame > 3) {
                backgroundFrame = 0;
            }
            lastBackgroundTick = currentTick;
        }

        if (game.level.isRadioOn()) {
            if (currentTick - lastRadioTick >= 300) {
                radioFrame++;
                if (radioFrame < 1 || radioFrame > 3) {
                    radioFrame = 1;
                }
                lastRadioTick = currentTick;
            }
        } else {
            radioFrame = 0;
        }

        SDL_SetRenderDrawColor(renderer, 18, 22, 28, 255);
        SDL_RenderClear(renderer);

        SDL_Rect backgroundRect = {0, -200, width, height};
        SDL_Rect fullScreen = {0, 0, width, height};

        SDL_RenderCopy(renderer, backgroundTextures[backgroundFrame], NULL, &backgroundRect);
        SDL_RenderCopy(renderer, interiorTexture, NULL, &fullScreen);
        SDL_RenderCopy(renderer, radioTextures[radioFrame], NULL, &fullScreen);

        SDL_Color textColor = {236, 240, 241, 255};
        int y = padding;
        for (const string& message : game.messages) {
            if (!renderTextBlock(renderer, font, message, textColor, padding, y, width - (padding * 2))) {
                SDL_StopTextInput();
                cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
                return -1;
            }

            // Move down after each line so the next message does not overlap it.
            int textHeight = 0;
            TTF_SizeUTF8(font, message.c_str(), NULL, &textHeight);
            y += textHeight + 18;
        }

        SDL_Color inputColor = {122, 214, 196, 255};
        if (!renderTextBlock(renderer, font, "> " + game.inputBuffer, inputColor, padding, height - 80, width - (padding * 2))) {
            SDL_StopTextInput();
            cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
            return -1;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 240);
    }

    SDL_StopTextInput();
    cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);

    return 0;
}