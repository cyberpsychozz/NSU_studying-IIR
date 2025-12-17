# Makefile for myls program

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -std=c99

# Target executable
TARGET = server client


# Default target
all: $(TARGET)

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

# Clean rule
clean:
	rm -f $(TARGET)

# Phony targets
.PHONY: all clean