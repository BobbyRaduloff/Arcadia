CC = g++
OPTS = -Wall -ggdb
NAME = caengine
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -Iimgui -Iimgui-sfml -lGL -std=c++17

all:
	$(CC) *.cpp imgui/*.cpp imgui-sfml/*.cpp -o $(NAME) $(LIBS) $(OPTS) 

run: all
	./$(NAME)

clean:
	rm $(NAME)