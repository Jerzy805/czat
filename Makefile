DIR = $(notdir $(CURDIR))

CC = g++

# ignorowanie unused result, bo chodzi o system, realnie nie ma znaczenia
CFLAGS = lobby_handler.cpp -pthread -O2 -Wno-unused-result

all: grouphost groupjoin chat call login

full: all copy

grouphost: grouphost.cpp
	$(CC) $< $(CFLAGS) -o $@

groupjoin: groupjoin.cpp
	$(CC) $< $(CFLAGS) -o $@

chat: chat.cpp
	$(CC) $< send_file.cpp get_file.cpp $(CFLAGS) -o $@

call: call.cpp
	$(CC) $< lobby_handler.cpp -o $@ -O2 -Wno-unused-result

login: login.cpp
	$(CC) $< -o $@ -O2 -Wno-unused-result

clean:
	rm -f *.o *.x a.out *~
	rm chat call grouphost groupjoin login

tar: clean
	(cd ../; tar -cvzf $(DIR).tar.gz $(DIR))

#przydatne na spk, jak się używa pogramu z dwóch rożnych miejsc na dysku
copy:
	cp call ..
	cp chat ..
	cp grouphost ..
	cp groupjoin ..
	cp login ..