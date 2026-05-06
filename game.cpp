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
    ChooseGenre,
    ChooseMode,
    EnterGuess,
    AskReplay
};

struct Button {
    SDL_Rect rect;
    string label;
    string value;
};

struct GameData {
    Level level;
    vector<string> messages;
    string currentAnswer;
    string inputBuffer;
    string selectedGenre;
    AppState state;
    bool quit;
    string mode;
    int score = 0;
};

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

static string normalizedGuess(const string& text)
{
    string normalized = trimCopy(text);
    transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(tolower(ch));
    });
    return normalized;
}

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

static bool renderButton(SDL_Renderer* renderer, TTF_Font* font, const Button& button, SDL_Color fillColor, SDL_Color textColor)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_RenderFillRect(renderer, &button.rect);

    SDL_SetRenderDrawColor(renderer, 244, 244, 244, 255);
    SDL_RenderDrawRect(renderer, &button.rect);

    int textWidth = 0;
    int textHeight = 0;
    if (TTF_SizeUTF8(font, button.label.c_str(), &textWidth, &textHeight) != 0) {
        return false;
    }

    const int textX = button.rect.x + (button.rect.w - textWidth) / 2;
    const int textY = button.rect.y + (button.rect.h - textHeight) / 2;
    return renderTextBlock(renderer, font, button.label, textColor, textX, textY, button.rect.w - 24);
}

static bool isInsideButton(const Button& button, int x, int y)
{
    return x >= button.rect.x && x <= button.rect.x + button.rect.w &&
           y >= button.rect.y && y <= button.rect.y + button.rect.h;
}

static vector<Button> buildGenreButtons(int width, int height)
{
    vector<Button> buttons;
    const int buttonWidth = 280;
    const int buttonHeight = 90;
    const int columnGap = 40;
    const int rowGap = 30;
    const int totalWidth = (buttonWidth * 2) + columnGap;
    const int startX = (width - totalWidth) / 2;
    const int startY = (height / 2) - 80;

    const char* genreLabels[] = {"Christmas", "RNB", "Rock", "Genre 4"};

    for (int index = 0; index < 4; ++index) {
        const int row = index / 2;
        const int col = index % 2;
        Button button;
        button.rect = {
            startX + (col * (buttonWidth + columnGap)),
            startY + (row * (buttonHeight + rowGap)),
            buttonWidth,
            buttonHeight
        };
        button.label = genreLabels[index];
        button.value = button.label;
        buttons.push_back(button);
    }

    return buttons;
}

static vector<Button> buildModeButtons(int width, int height)
{
    vector<Button> buttons;
    const int buttonWidth = 240;
    const int buttonHeight = 90;
    const int gap = 30;
    const int totalWidth = (buttonWidth * 3) + (gap * 2);
    const int startX = (width - totalWidth) / 2;
    const int y = (height / 2) - 10;

    const char* labels[] = {"Lyrics", "Melody", "Rhythm"};
    const char* values[] = {"lyrics", "melody", "rhythm"};

    for (int index = 0; index < 3; ++index) {
        Button button;
        button.rect = {startX + (index * (buttonWidth + gap)), y, buttonWidth, buttonHeight};
        button.label = labels[index];
        button.value = values[index];
        buttons.push_back(button);
    }

    return buttons;
}

static vector<Button> buildReplayButtons(int width, int height)
{
    vector<Button> buttons;
    const int buttonWidth = 180;
    const int buttonHeight = 74;
    const int gap = 30;
    const int totalWidth = (buttonWidth * 2) + gap;
    const int startX = (width - totalWidth) / 2;
    const int y = height - 190;

    Button yesButton;
    yesButton.rect = {startX, y, buttonWidth, buttonHeight};
    yesButton.label = "Play Again";
    yesButton.value = "play-again";
    buttons.push_back(yesButton);

    Button noButton;
    noButton.rect = {startX + buttonWidth + gap, y, buttonWidth, buttonHeight};
    noButton.label = "Quit";
    noButton.value = "quit";
    buttons.push_back(noButton);

    return buttons;
}

static void showGenreMenu(GameData& game)
{
    game.level.stopAudio();
    game.inputBuffer.clear();
    game.messages.clear();
    pushMessage(game.messages, "Choose a genre to start.");
    //pushMessage(game.messages, "These are placeholder labels for your team to rename.");
    game.state = AppState::ChooseGenre;
}

static void showModeMenu(GameData& game)
{
    game.level.stopAudio();
    game.inputBuffer.clear();
    game.messages.clear();
    pushMessage(game.messages, "Selected " + game.selectedGenre + ".");
    pushMessage(game.messages, "Choose whether to guess by lyrics, melody, or rhythm.");
    game.state = AppState::ChooseMode;
}

static void beginRound(GameData& game, const string& nextMode)
{
    string prompt;
    game.currentAnswer.clear();
    game.mode = nextMode;
    game.inputBuffer.clear();
    game.messages.clear();
    pushMessage(game.messages, "Genre: " + game.selectedGenre);

    if (game.mode == "lyrics") {
        if (game.level.lyrics(game.selectedGenre, prompt, game.currentAnswer) != 0) {
            pushMessage(game.messages, prompt);
            game.quit = true;
            return;
        }
    } else if (game.mode == "rhythm" || game.mode == "melody") {
        if (game.level.playMusic(game.mode, game.selectedGenre, prompt, game.currentAnswer) != 0) {
            pushMessage(game.messages, prompt);
            game.quit = true;
            return;
        }
    }

    pushMessage(game.messages, prompt);
    if (game.mode == "lyrics") {
        pushMessage(game.messages, "Type the song title and press Enter.");
    } else {
        pushMessage(game.messages, "Listen, type the song title, and press Enter.");
    }
    game.state = AppState::EnterGuess;
}

static void finishRound(GameData& game, const string& rawInput)
{
    const string value = trimCopy(rawInput);
    if (value.empty()) {
        return;
    }

    game.level.stopAudio();
    if (normalizedGuess(value) == normalizedGuess(game.currentAnswer)) {
        pushMessage(game.messages, "You guessed correctly!");
        game.score += 10;
    } else {
        pushMessage(game.messages, "You guessed wrong.");
        pushMessage(game.messages, "Answer: " + game.currentAnswer);
        game.score -= 5;
    }

    pushMessage(game.messages, "Score: " + to_string(game.score));
    pushMessage(game.messages, "Choose what to do next.");
    game.state = AppState::AskReplay;
    game.inputBuffer.clear();
}

static void handleMenuClick(GameData& game, int mouseX, int mouseY, const vector<Button>& buttons)
{
    for (const Button& button : buttons) {
        if (!isInsideButton(button, mouseX, mouseY)) {
            continue;
        }

        if (game.state == AppState::ChooseGenre) {
            game.selectedGenre = button.value;
            showModeMenu(game);
        } else if (game.state == AppState::ChooseMode) {
            beginRound(game, button.value);
        } else if (game.state == AppState::AskReplay) {
            if (button.value == "play-again") {
                showGenreMenu(game);
            } else if (button.value == "quit") {
                game.quit = true;
            }
        }
        return;
    }
}

int runGame(SDL_Renderer* renderer, TTF_Font* font, int width, int height)
{
    GameData game;
    game.state = AppState::ChooseGenre;
    game.quit = false;
    game.score = 0;

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

    showGenreMenu(game);

    int backgroundFrame = 0;
    int radioFrame = 0;
    Uint32 lastBackgroundTick = SDL_GetTicks();
    Uint32 lastRadioTick = SDL_GetTicks();

    SDL_Event e;
    SDL_StartTextInput();

    while (!game.quit) {
        const vector<Button> genreButtons = buildGenreButtons(width, height);
        const vector<Button> modeButtons = buildModeButtons(width, height);
        const vector<Button> replayButtons = buildReplayButtons(width, height);

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                game.quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                if (game.state == AppState::ChooseGenre) {
                    handleMenuClick(game, e.button.x, e.button.y, genreButtons);
                } else if (game.state == AppState::ChooseMode) {
                    handleMenuClick(game, e.button.x, e.button.y, modeButtons);
                } else if (game.state == AppState::AskReplay) {
                    handleMenuClick(game, e.button.x, e.button.y, replayButtons);
                }
            } else if (e.type == SDL_TEXTINPUT) {
                if (game.state == AppState::EnterGuess) {
                    game.inputBuffer += e.text.text;
                }
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    if (game.state == AppState::ChooseMode || game.state == AppState::AskReplay) {
                        showGenreMenu(game);
                    } else {
                        game.quit = true;
                    }
                } else if (game.state == AppState::EnterGuess) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && !game.inputBuffer.empty()) {
                        game.inputBuffer.pop_back();
                    } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                        finishRound(game, game.inputBuffer);
                    }
                }
            }
        }

        const Uint32 currentTick = SDL_GetTicks();

        if (currentTick - lastBackgroundTick >= 120) {
            backgroundFrame = (backgroundFrame + 1) % 4;
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

        SDL_Color headingColor = {249, 233, 166, 255};
        SDL_Color textColor = {236, 240, 241, 255};
        SDL_Color inputColor = {122, 214, 196, 255};
        SDL_Color buttonTextColor = {244, 244, 244, 255};

        if (game.state == AppState::ChooseGenre) {
            if (!renderTextBlock(renderer, font, "Main Menu", headingColor, padding, 70, width - (padding * 2)) ||
                !renderTextBlock(renderer, font, "Click a placeholder genre.", textColor, padding, 120, width - (padding * 2))) {
                SDL_StopTextInput();
                cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
                return -1;
            }

            for (const Button& button : genreButtons) {
                if (!renderButton(renderer, font, button, {28, 87, 122, 220}, buttonTextColor)) {
                    SDL_StopTextInput();
                    cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
                    return -1;
                }
            }
        } else if (game.state == AppState::ChooseMode) {
            if (!renderTextBlock(renderer, font, "Genre Selected", headingColor, padding, 70, width - (padding * 2)) ||
                !renderTextBlock(renderer, font, game.selectedGenre, textColor, padding, 120, width - (padding * 2)) ||
                !renderTextBlock(renderer, font, "Click how the player should guess the song.", textColor, padding, 170, width - (padding * 2))) {
                SDL_StopTextInput();
                cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
                return -1;
            }

            for (const Button& button : modeButtons) {
                if (!renderButton(renderer, font, button, {123, 63, 0, 220}, buttonTextColor)) {
                    SDL_StopTextInput();
                    cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
                    return -1;
                }
            }
        } else {
            int y = padding;
            for (const string& message : game.messages) {
                if (!renderTextBlock(renderer, font, message, textColor, padding, y, width - (padding * 2))) {
                    SDL_StopTextInput();
                    cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
                    return -1;
                }

                int textHeight = 0;
                TTF_SizeUTF8(font, message.c_str(), NULL, &textHeight);
                y += textHeight + 18;
            }

            if (game.state == AppState::EnterGuess) {
                if (!renderTextBlock(renderer, font, "> " + game.inputBuffer, inputColor, padding, height - 80, width - (padding * 2))) {
                    SDL_StopTextInput();
                    cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
                    return -1;
                }
            } else if (game.state == AppState::AskReplay) {
                for (const Button& button : replayButtons) {
                    if (!renderButton(renderer, font, button, {42, 92, 53, 220}, buttonTextColor)) {
                        SDL_StopTextInput();
                        cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);
                        return -1;
                    }
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 240);
    }

    SDL_StopTextInput();
    cleanupVisuals(backgroundTextures, radioTextures, interiorTexture);

    return 0;
}
