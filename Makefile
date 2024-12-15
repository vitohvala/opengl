CC=clang
LIBS=`pkg-config glfw3 --libs` -lm
FLAGS=`pkg-config glfw3 --cflags` -Wall -Wextra -g
INCDIR=-I/home/vito/git/opengl/include 
TARGET=src/main.c src/glad.c
BIN=exe

all:
	$(CC) -o $(BIN) $(TARGET) $(FLAGS) $(LIBS) $(INCDIR)

run: all
	./$(BIN)
