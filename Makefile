CC = gcc
CFLAGS = -Wall -Wextra -pedantic -pthread
TARGET = czat
SRC = chat.c call.c commands.c file_send.c
# tu wyzej po spacji dopisywac pliki do skompilowania
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
