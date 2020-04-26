CC = g++
OPTS = -Wall -ggdb
NAME = caengine
LIBS = -lsfml-graphics -lsfml-window -lsfml-system  -std=c++17

all:
	$(CC) *.cpp -o $(NAME) $(LIBS) $(OPTS) 

run: all
	./$(NAME)

clean:
	rm $(NAME)