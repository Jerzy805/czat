#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX 256
void commands(char* input){
    if(strcmp(input, "help") == 0 || strcmp(input, "h") == 0 || strcmp(input, "?") == 0){
        printf("Dostępne komendy:\n");
        printf("help - wyświetla tę pomoc\n");
        printf("plik - przesyła plik do drugiego użytkownika\n");
    }
    else if(strcmp(input, "plik") == 0){
        system("clear");
    }
    else {
        printf("Nieznana komenda. Wpisz 'help' aby zobaczyć dostępne komendy.\n");
    }

}