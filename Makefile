PROJ = simple
CXX = g++

SDL_PATH = /mingw64

FLAGS = -I"$(SDL_PATH)/include/SDL2"
LIBS  = -L"$(SDL_PATH)/lib" -lmingw32 -lSDL2main -lSDL2

run: $(PROJ).exe
	./$<

$(PROJ).exe: $(PROJ).cpp Character.hpp Game.hpp MediaManager.hpp MyGame.hpp
	$(CXX) $(FLAGS) $< -o $@ $(LIBS)