DIR = $(notdir $(CURDIR))

CC = g++

CFLAGS = -pthread

all: grouphost groupjoin chat call

full: all copy

grouphost: grouphost.cpp
	$(CC) $< $(CFLAGS) -o $@

groupjoin: groupjoin.cpp
	$(CC) $< $(CFLAGS) -o $@

chat: chat.cpp
	$(CC) $< send_file.cpp get_file.cpp $(CFLAGS) -o $@

call: call.cpp
	$(CC) $< -o $@

clean:
	rm -f *.o *.x a.out *~
	rm chat call grouphost groupjoin

tar: clean
	(cd ../; tar -cvzf $(DIR).tar.gz $(DIR))

#przydatne na spk, jak się używa pogramu z dwóch rożnych miejsc na dysku
copy:
	cp call ..
	cp chat ..
	cp grouphost ..
	cp groupjoin ..