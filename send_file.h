#ifndef SEND_FILE_H
#define SEND_FILE_H

void send_file(const char *my_file, const char *filename, char id[30]); // pierwszy argument to plik, w którym zapisywane są własne wiadomości
// w obecnej postaci funkcja kopiuje plik do folderu /tmp/ i nadaje prawo czytania rozmówcy
#endif