PROJ=main
SRCS=main.cpp screen.cpp
OBJS=$(SRCS:.cpp=.o)
FLAGS=$(shell sdl2-config --cflags)
LIBS=$(shell sdl2-config --libs)
TTF_LIBS=-lSDL2_ttf

run: ${PROJ}.exe
	./$<

${PROJ}.exe: ${OBJS}
	g++ ${FLAGS} $^ -o $@ ${LIBS} ${TTF_LIBS}

%.o: %.cpp
	g++ ${FLAGS} -c $< -o $@

clean:
	rm -f ${OBJS} ${PROJ}.exe
