#include "game.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include <SDL.h>
#include <SDL_surface.h>

#include "levels.h"
#include <iostream>
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


    // Maleia: Album cover popup
    SDL_Texture* albumCoverTexture = nullptr;
};

// Paul: Declarations for visuals
static SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& fileName, bool useColorKey);
static bool isChristmasGenre(const string& genre);
static bool isRnbGenre(const string& genre);
static bool isRockGenre(const string& genre);

// Paul: Sprite class
class Sprite {
    vector<SDL_Texture*> textures;
    SDL_Rect dst;
    int which;
    int totalMillis;
    int frameTime;

// Paul: Sprite methods
public:
    Sprite()
    {
        dst = {0, 0, 0, 0};
        which = 0;
        totalMillis = 0;
        frameTime = 250;
    }

    // Paul: Load sprite image files
    bool load(SDL_Renderer* renderer, const vector<string>& fileNames,
              int x, int y, int w, int h, bool useColorKey, int newFrameTime)
    {
        dst = {x, y, w, h};
        frameTime = newFrameTime;
        which = 0;
        totalMillis = 0;

        for (const string& name : fileNames) {
            SDL_Texture* texture = loadTexture(renderer, "resources/images/" + name, useColorKey);
            if (texture == NULL) {
                return false;
            }
            textures.push_back(texture);
        }

        return true;
    }

    // Paul: Update sprite frames
    void update(float dt)
    {
        if (textures.size() <= 1) {
            return;
        }

        totalMillis += static_cast<int>(dt * 1000);
        while (totalMillis >= frameTime) {
            which++;
            if (which >= static_cast<int>(textures.size())) {
                which = 0;
            }
            totalMillis -= frameTime;
        }
    }

    // Paul: Set sprite back to its idle frame
    void idle()
    {
        which = 0;
        totalMillis = 0;
    }

    // Paul: Draw the current sprite frame
    void draw(SDL_Renderer* renderer)
    {
        if (textures.empty()) {
            return;
        }

        if (which < 0 || which >= static_cast<int>(textures.size())) {
            which = 0;
        }

        SDL_RenderCopy(renderer, textures[which], NULL, &dst);
    }

    ~Sprite()
    {
        for (SDL_Texture* texture : textures) {
            if (texture != NULL) {
                SDL_DestroyTexture(texture);
            }
        }
        textures.clear();
    }
};

// Paul: Artwork grouping class
class Artwork {
    Sprite titleBackground;
    Sprite titleName;
    Sprite record;

    Sprite rnbBackground;
    Sprite rnbInterior;
    Sprite rnbRadio;

    Sprite christmasBackground;
    Sprite deer;
    Sprite christmasInterior;
    Sprite christmasRadio;

    Sprite rockBackground;
    Sprite rockInterior;
    Sprite rockRadio;

public:
    // Paul: Load BMP files
    bool setup(SDL_Renderer* renderer, int width, int height)
    {
        if (!titleBackground.load(renderer,
            {"title01.bmp", "title02.bmp", "title03.bmp", "title04.bmp"},
            0, 0, width, height, false, 400)) {
            return false;
        }

        if (!titleName.load(renderer, {"name01.bmp"},
            0, 0, width, height, true, 400)) {
            return false;
        }

        if (!record.load(renderer,
            {"record01.bmp", "record02.bmp", "record03.bmp", "record04.bmp"},
            0, 0, width, height, true, 400)) {
            return false;
        }

        if (!rnbBackground.load(renderer,
            {"background0.bmp", "background1.bmp", "background2.bmp", "background3.bmp"},
            0, -200, width, height, false, 120)) {
            return false;
        }

        if (!rnbInterior.load(renderer, {"interior0.bmp"},
            0, 0, width, height, true, 250)) {
            return false;
        }

        if (!rnbRadio.load(renderer,
            {"radio0.bmp", "radio1.bmp", "radio2.bmp", "radio3.bmp", "radio4.bmp"},
            0, 0, width, height, true, 300)) {
            return false;
        }

        if (!christmasBackground.load(renderer, {"cbackground01.bmp"},
            0, -50, width, height, false, 250)) {
            return false;
        }

        if (!deer.load(renderer,
            {"deer01.bmp", "deer02.bmp", "deer03.bmp", "deer04.bmp"},
            0, 0, width, height, true, 140)) {
            return false;
        }

        if (!christmasInterior.load(renderer, {"cinterior01.bmp"},
            0, 0, width, height, true, 250)) {
            return false;
        }

        if (!christmasRadio.load(renderer,
            {"cradio00.bmp", "cradio01.bmp", "cradio02.bmp", "cradio03.bmp"},
            0, 0, width, height, true, 300)) {
            return false;
        }

        if (!rockBackground.load(renderer,
            {"rbackground01.bmp", "rbackground02.bmp", "rbackground03.bmp"},
            0, -180, width, height, false, 400)) {
            return false;
        }

        if (!rockInterior.load(renderer, {"rinterior01.bmp"},
            0, 0, width, height, true, 250)) {
            return false;
        }

        if (!rockRadio.load(renderer,
            {"Rradio00.bmp", "Rradio01.bmp", "Rradio02.bmp", "Rradio03.bmp"},
            0, 0, width, height, true, 300)) {
            return false;
        }

        return true;
    }

    // Paul: Update visual frames
    void update(float dt, bool radioOn)
    {
        titleBackground.update(dt);
        record.update(dt);
        rnbBackground.update(dt);
        deer.update(dt);
        rockBackground.update(dt);

        if (radioOn) {
            rnbRadio.update(dt);
            christmasRadio.update(dt);
            rockRadio.update(dt);
        } else {
            rnbRadio.idle();
            christmasRadio.idle();
            rockRadio.idle();
        }
    }

    // Paul: Draw proper visuals based on selectedGenre
    void draw(SDL_Renderer* renderer, const GameData& game)
    {
        if (game.state == AppState::ChooseGenre) {
            titleBackground.draw(renderer);
            titleName.draw(renderer);
            record.draw(renderer);
        } else if (isChristmasGenre(game.selectedGenre)) {
            christmasBackground.draw(renderer);
            deer.draw(renderer);
            christmasInterior.draw(renderer);
            christmasRadio.draw(renderer);
        } else if (isRnbGenre(game.selectedGenre)) {
            rnbBackground.draw(renderer);
            rnbInterior.draw(renderer);
            rnbRadio.draw(renderer);
        } else if (isRockGenre(game.selectedGenre)) {
            rockBackground.draw(renderer);
            rockInterior.draw(renderer);
            rockRadio.draw(renderer);
        }
    }
};

// Paul: Load BMP files and apply 255 255 255 color key for transparency
static SDL_Texture* loadTexture(SDL_Renderer* renderer, const string& fileName, bool useColorKey)
{
    SDL_Surface* surface = SDL_LoadBMP(fileName.c_str());
    if (surface == NULL) {
        return NULL;
    }

    if (useColorKey) {
        Uint32 key = SDL_MapRGB(surface->format, 255, 255, 255);
        SDL_SetColorKey(surface, SDL_TRUE, key);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Paul: Visual genre checks
static bool isChristmasGenre(const string& genre)
{
    return genre == "Christmas";
}

static bool isRnbGenre(const string& genre)
{
    return genre == "RNB";
}

static bool isRockGenre(const string& genre)
{
    return genre == "Rock";
}


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

static bool renderMenuLabelOnly(SDL_Renderer* renderer, TTF_Font* font, const Button& button, SDL_Color textColor)
{
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

    const char* genreLabels[] = {"Christmas", "RNB", "Rock", "Quit"};
    const char* genreValues[] = {"Christmas", "RNB", "Rock", "exit"};

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
        button.value = genreValues[index];
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
    noButton.label = "Main Menu";
    noButton.value = "main-menu";
    buttons.push_back(noButton);

    return buttons;
}

// Destroy album cover texture and clear the pointer
static void clearAlbumCover(GameData& game)
{
    if (game.albumCoverTexture) {
        SDL_DestroyTexture(game.albumCoverTexture);
        game.albumCoverTexture = nullptr;
    }
}

// Draw the album cover popup centered on screen, above the replay buttons
static void drawAlbumCoverPopup(SDL_Renderer* renderer, TTF_Font* font, GameData& game, int width, int height)
{
    if (!game.albumCoverTexture) {
        return;
    }

    const int coverSize = 280;
    const int coverX = (width - coverSize) / 2;

    // Position the cover so it sits comfortably above the replay buttons (which are at height-190)
    // Leave room for the result text above the cover
    const int coverY = height - 190 - coverSize - 20;

    // Dark semi-transparent backdrop with a border
    const int borderPad = 10;
    SDL_Rect backdrop = {
        coverX - borderPad,
        coverY - borderPad,
        coverSize + (borderPad * 2),
        coverSize + (borderPad * 2)
    };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &backdrop);

    // Bright border around the cover
    SDL_SetRenderDrawColor(renderer, 249, 233, 166, 255);
    SDL_RenderDrawRect(renderer, &backdrop);

    // Draw an inner border for a polished look
    SDL_Rect innerBorder = {
        backdrop.x + 2,
        backdrop.y + 2,
        backdrop.w - 4,
        backdrop.h - 4
    };
    SDL_SetRenderDrawColor(renderer, 180, 160, 100, 180);
    SDL_RenderDrawRect(renderer, &innerBorder);

    // The cover image itself
    SDL_Rect coverDst = {coverX, coverY, coverSize, coverSize};
    SDL_RenderCopy(renderer, game.albumCoverTexture, NULL, &coverDst);
}

static void showGenreMenu(GameData& game)
{
    game.level.stopAudio();
    game.inputBuffer.clear();
    game.messages.clear();
    clearAlbumCover(game);
    pushMessage(game.messages, "Choose a genre to start.");
    game.state = AppState::ChooseGenre;
}

static void showModeMenu(GameData& game)
{
    game.level.stopAudio();
    game.inputBuffer.clear();
    game.messages.clear();
    clearAlbumCover(game);
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
    clearAlbumCover(game);
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

static void finishRound(GameData& game, const string& rawInput, SDL_Renderer* renderer)
{
    const string value = trimCopy(rawInput);
    if (value.empty()) {
        return;
    }

    game.level.stopAudio();

    // Load the album cover for the current song
    clearAlbumCover(game);
    const string coverPath = game.level.coverImagePath();
    if (!coverPath.empty()) {
        // loadTexture returns NULL silently if the file doesn't exist — that's fine,
        // the popup simply won't show if no cover BMP is found
       cout << "Cover path: " << coverPath << endl;
        game.albumCoverTexture = loadTexture(renderer, coverPath, false);
    }

    if (normalizedGuess(value) == normalizedGuess(game.currentAnswer)) {
        int points = 0;
        if (game.mode == "rhythm") {
            points = 30;
        } else if (game.mode == "melody") {
            points = 20;
        } else if (game.mode == "lyrics") {
            points = 10;
        }

        pushMessage(game.messages, "You guessed correctly!");
        pushMessage(game.messages, string("+ ") + to_string(points) + " points.");
        game.score += points;
    } else {
        pushMessage(game.messages, "You guessed wrong.");
        pushMessage(game.messages, "Answer: " + game.currentAnswer);
        game.score -= 10;
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
            if (button.value == "exit") {
                game.quit = true;
            } else {
                game.selectedGenre = button.value;
                showModeMenu(game);
            }
        } else if (game.state == AppState::ChooseMode) {
            beginRound(game, button.value);
        } else if (game.state == AppState::AskReplay) {
            if (button.value == "play-again") {
                showGenreMenu(game);
            } else if (button.value == "main-menu") {
                showGenreMenu(game);
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
    game.albumCoverTexture = nullptr;

 

    // Paul: Create and load artwork
    Artwork artwork;
    if (!artwork.setup(renderer, width, height)) {
        return -1;
    }

    

    showGenreMenu(game);

    // Paul: animation timing
    Uint32 lastTick = SDL_GetTicks();

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
                        finishRound(game, game.inputBuffer, renderer);
                    }
                }
            }
        }

        // Paul: Update animation frames
        Uint32 currentTick = SDL_GetTicks();
        float dt = static_cast<float>(currentTick - lastTick) / 1000.0f;
        lastTick = currentTick;

        artwork.update(dt, game.level.isRadioOn());
       

        SDL_SetRenderDrawColor(renderer, 18, 22, 28, 255);
        SDL_RenderClear(renderer);

        // Paul: Draw genre images
        artwork.draw(renderer, game);

        
        // Draw album cover popup on top of background, behind text and buttons
        if (game.state == AppState::AskReplay) {
            drawAlbumCoverPopup(renderer, font, game, width, height);
        }

        SDL_Color headingColor = {249, 233, 166, 255};
        SDL_Color textColor = {236, 240, 241, 255};
        SDL_Color inputColor = {122, 214, 196, 255};
        SDL_Color buttonTextColor = {244, 244, 244, 255};

        string scoreText = "Score: " + to_string(game.score);
        int scoreTextWidth = 0;
        int scoreTextHeight = 0;
        TTF_SizeUTF8(font, scoreText.c_str(), &scoreTextWidth, &scoreTextHeight);
        if (!renderTextBlock(renderer, font, scoreText, headingColor, width - padding - scoreTextWidth, padding, scoreTextWidth)) {
            SDL_StopTextInput();
            clearAlbumCover(game);
            return -1;
        }

        if (game.state == AppState::ChooseGenre) {
            for (const Button& button : genreButtons) {
                if (!renderMenuLabelOnly(renderer, font, button, buttonTextColor)) {
                    SDL_StopTextInput();
                    clearAlbumCover(game);
                    return -1;
                }
            }
        } else if (game.state == AppState::ChooseMode) {
            if (!renderTextBlock(renderer, font, "Genre Selected", headingColor, padding, 70, width - (padding * 2)) ||
                !renderTextBlock(renderer, font, game.selectedGenre, textColor, padding, 120, width - (padding * 2)) ||
                !renderTextBlock(renderer, font, "Click how the player should guess the song.", textColor, padding, 170, width - (padding * 2))) {
                SDL_StopTextInput();
                clearAlbumCover(game);
                return -1;
            }

            for (const Button& button : modeButtons) {
                if (!renderButton(renderer, font, button, {123, 63, 0, 220}, buttonTextColor)) {
                    SDL_StopTextInput();
                    clearAlbumCover(game);
                    return -1;
                }
            }
        } else {
            int y = padding;
            for (const string& message : game.messages) {
                if (!renderTextBlock(renderer, font, message, textColor, padding, y, width - (padding * 2))) {
                    SDL_StopTextInput();
                    clearAlbumCover(game);
                    return -1;
                }

                int textHeight = 0;
                TTF_SizeUTF8(font, message.c_str(), NULL, &textHeight);
                y += textHeight + 18;
            }

            if (game.state == AppState::EnterGuess) {
                if (!renderTextBlock(renderer, font, "> " + game.inputBuffer, inputColor, padding, height - 80, width - (padding * 2))) {
                    SDL_StopTextInput();
                    clearAlbumCover(game);
                    return -1;
                }
            } else if (game.state == AppState::AskReplay) {
                for (const Button& button : replayButtons) {
                    if (!renderButton(renderer, font, button, {42, 92, 53, 220}, buttonTextColor)) {
                        SDL_StopTextInput();
                        clearAlbumCover(game);
                        return -1;
                    }
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / 240);
    }

    SDL_StopTextInput();
    clearAlbumCover(game);
    return 0;
}
