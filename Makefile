CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lncurses
TARGET = game_of_life
SRC = game_of_life.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

install:
	sudo apt-get install libncurses5-dev

.PHONY: all clean run install