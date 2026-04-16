CC = gcc
CFLAGS = -Wall -Wextra -pedantic -pthread

CHAT_TARGET = chat
CALL_TARGET = call

CHAT_SRC = chat.c commands.c file_send.c
CALL_SRC = call.c

CHAT_OBJ = $(CHAT_SRC:.c=.o)
CALL_OBJ = $(CALL_SRC:.c=.o)

all: $(CHAT_TARGET) $(CALL_TARGET)

$(CHAT_TARGET): $(CHAT_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CHAT_OBJ)

$(CALL_TARGET): $(CALL_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CALL_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(CHAT_OBJ) $(CALL_OBJ) $(CHAT_TARGET) $(CALL_TARGET)

.PHONY: all clean
