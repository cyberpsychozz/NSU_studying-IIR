CC = gcc
CFLAGS = -Wall
TARGET = sol
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)


.PHONY: all clean
